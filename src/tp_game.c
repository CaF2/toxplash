#include "tp_game.h"

GArray *GLOBAL_ROOMS=NULL;

TPGameRoom *tp_game_room_init(TPGameRoom *self, guint32 first_player_id, gchar *room_name)
{
	memset(self,'\0',sizeof(TPGameRoom));

	self->room_name=g_strdup(room_name);
	self->state=TP_GAME_ROOM_STATE_WAIT_PLAYERS;
	self->round=0;
	self->players=g_array_new(FALSE,FALSE,sizeof(TPPlayer));
	
	TPPlayer first_player;
	tp_player_init(&first_player,first_player_id);
	
	g_array_append_val(self->players,first_player);
	
	return self;
}

TPGameRoom *tp_game_room_new(guint32 first_player_id, gchar *room_name)
{
	return tp_game_room_init(g_malloc(sizeof(TPGameRoom)), first_player_id, room_name);
}

gint tp_game_room_add(guint32 first_player_id, gchar *room_name)
{
	if(GLOBAL_ROOMS==NULL)
	{
		GLOBAL_ROOMS=g_array_new(FALSE,FALSE,sizeof(TPGameRoom));
	}
	
	TPGameRoom room;
	tp_game_room_init(&room,first_player_id,room_name);
	
	g_array_append_val(GLOBAL_ROOMS,room);
	
	return 0;
}

TPGameRoom *tp_game_room_find_from_name(gchar *room_name)
{
	if(GLOBAL_ROOMS)
	{
		TPGameRoom *rooms=(TPGameRoom*)GLOBAL_ROOMS->data;
		for(gint i=0;i<GLOBAL_ROOMS->len;i++)
		{	
			if(strcmp(rooms[i].room_name,room_name)==0)
			{
				return &rooms[i];
			}
		}
	}
	
	return NULL;
}

TPGameRoom *tp_game_room_find_from_user(guint32 player_id)
{
	if(GLOBAL_ROOMS)
	{
		TPGameRoom *rooms=(TPGameRoom*)GLOBAL_ROOMS->data;
		for(gint i=0;i<GLOBAL_ROOMS->len;i++)
		{
			TPPlayer *players=(TPPlayer*)rooms[i].players->data;
			for(gint ii=0;ii<rooms[i].players->len;ii++)
			{
				if(players[ii].player_id==player_id)
				{
					return &rooms[i];
				}
			}
		}
	}
	
	return NULL;
}

TPPlayer *tp_game_room_find_user(TPGameRoom *self, guint32 player_id)
{
	TPPlayer *players=(TPPlayer*)self->players->data;
	for(gint ii=0;ii<self->players->len;ii++)
	{
		if(players[ii].player_id==player_id)
		{
			return &players[ii];
		}
	}
	
	return NULL;
}

gint tp_game_room_add_user(TPGameRoom *self, guint32 player_id)
{
	if(self==NULL)
	{
		return 1;
	}
	else
	{
		TPPlayer *players=(TPPlayer*)self->players->data;
		for(gint ii=0;ii<self->players->len;ii++)
		{
			if(players[ii].player_id==player_id)
			{
				//already exists
				return 1;
			}
		}
		
		TPPlayer curr_player;
		
		tp_player_init(&curr_player,player_id);
		
		g_array_append_val(self->players,curr_player);
		
		printf("Added user %d to room %s\n",player_id,self->room_name);
		
		gchar *user_list=g_string_free(tp_game_room_dump_players(self),FALSE);
		
		printf("%s",user_list);
		
		g_free(user_list);
		
		return 0;
	}
}

///TODO IMPROVE (wont remove room if 0 users)
gint tp_game_room_remove_user(TPGameRoom *self, guint32 player_id)
{
	if(self==NULL)
	{
		return 1;
	}
	else
	{
		TPPlayer *players=(TPPlayer*)self->players->data;
		
		int already_exist=0;
		gint ii=0;
		while(ii<self->players->len)
		{
			if(players[ii].player_id==player_id)
			{
				already_exist=1;
				break;
			}
			ii++;
		}
		
		if(already_exist==0)
		{
			//not in the room
			return 1;
		}
		
		TPPlayer curr_player;
		
		tp_player_init(&curr_player,player_id);
		
		g_array_remove_index(self->players,ii);
		
		printf("removed user %d from the room %s\n",player_id,self->room_name);
		
		g_autofree gchar *user_list=g_string_free(tp_game_room_dump_players(self),FALSE);
		
		printf("%s",user_list);
		
		g_free(user_list);
		
		return 0;
	}
}

gint tp_game_room_add_user_from_room_name(gchar *room_name, guint32 player_id)
{
	if(GLOBAL_ROOMS==NULL)
	{
		return tp_game_room_add(player_id,room_name);
	}
	else
	{
		TPGameRoom *suggested_room=tp_game_room_find_from_name(room_name);
		
		//on fail add room
		if(tp_game_room_add_user(suggested_room,player_id)!=0)
		{
			return tp_game_room_add(player_id,room_name);
		}
	}
	
	return 0;
}

int tp_game_room_start_game(TPGameRoom *self)
{
	TPPlayer *players=(TPPlayer*)self->players->data;
	for(gint ii=0;ii<self->players->len;ii++)
	{
		//if(players[ii].player_id==player_id)
		//{
		//	return &rooms[i];
		//}
	}
	
	return 0;
}

GString *tp_game_room_dump_players(TPGameRoom *self)
{
	GString *output=g_string_sized_new(100);
	
	TPPlayer *players=(TPPlayer*)self->players->data;
	for(gint i=0;i<self->players->len;i++)
	{
		g_string_append_printf(output,"%s (ID=%d)\n","TODO",players[i].player_id);
	}
	
	return output;
}

/**
	two rounds a la 60 sec, 2 questions (both can be answered at the same time).
	In the end one question all will answer
*/
void *tp_game_play(void* argument)
{
	void **input=argument;

	TPGameRoom *self=input[0];
	Tox *tox=input[1];
	
	free(input);
	
	GTimer *timer=g_timer_new();
	
	TPPlayer *players=(TPPlayer*)self->players->data;
	for(gint ii=0;ii<self->players->len;ii++)
	{
		guint32 curr_player_id=players[ii].player_id;
		
		gchar *str=g_strdup_printf("==%s==",tp_qa_get_question((((TPQa*)players[ii].qa->data)[0]).question));
	
		tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
	
		g_free(str);
	}
	
	g_timer_start(timer);
	
	int total_rounds=2;
	
	for(self->round=0;self->round<total_rounds;self->round++)
	{
		for(gint elapsed=10;elapsed>=0;elapsed--)
		{
			TPPlayer *players=(TPPlayer*)self->players->data;
			for(gint ii=0;ii<self->players->len;ii++)
			{
				TPPlayer *curr_player=&players[ii];
			
				guint32 curr_player_id=curr_player->player_id;
				
				TPQa *qa=(TPQa*)curr_player->qa->data;
				
				if(curr_player->curr_qa<total_rounds)
				{
					gchar *str=g_strdup_printf("== %d ==",elapsed);
		
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
					
					g_free(str);
					
					str=g_strdup_printf("== %s %d ==",tp_qa_get_question(qa[curr_player->curr_qa].question),players[ii].qa->len);
					
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
	
					g_free(str);
				}
				else
				{
					gchar *str=g_strdup_printf("== Waiting for other players. %d sec left ... ==",elapsed);
	
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
	
					g_free(str);
				}
			}
		
			g_usleep(1000000);
		}
		
		g_usleep(1000000);
		
		tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,"Voting phase, get ready to vote!");
		
		g_usleep(1000000);
		
		TPPlayer *players=(TPPlayer*)self->players->data;
		
		TPPlayer *vote_order=&players[0];
		
		TPQa *vote_qa=(TPQa*)vote_order->qa->data;
		
		for(gint question=0;question<2;question++)
		{
			for(gint elapsed=30;elapsed>=0;elapsed--)
			{
				gchar *str=g_strdup_printf("== %d ==",elapsed);
	
				tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
		
				g_free(str);
		
				str=g_strdup_printf("== %s ==",tp_qa_get_question(vote_qa[0].question));
		
				tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
		
				g_free(str);
			
				TPPlayer *players=(TPPlayer*)self->players->data;
				for(gint ii=0;ii<self->players->len;ii++)
				{
					TPPlayer *curr_player=&players[ii];
			
					guint32 curr_player_id=curr_player->player_id;
					
					TPQa *qa=(TPQa*)curr_player->qa->data;
					
					str=g_strdup_printf("%c) %s",'A'+ii,qa[question].answer);
					
					tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
					
					g_free(str);
				}	
				
				g_usleep(1000000);
			}
		}
	}
	
	tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,"Times up!");
	
	return NULL;
}

void tp_game_answer(TPGameRoom *self, Tox *tox, uint32_t friend_number, const uint8_t *message, size_t length)
{
	TPPlayer *player=tp_game_room_find_user(self,friend_number);
	
	if(player)
	{
		TPQa *qas=(TPQa*)player->qa->data;
		
		if(player->curr_qa<2)
		{
			TPQa *qa=&qas[player->curr_qa];
		
			if(!qa->answer)
			{
				qa->answer=strndup(message,length);
				
				player->curr_qa++;
				
				if(player->curr_qa==2)
				{
					tp_server_send_message_str(tox,friend_number,TOX_MESSAGE_TYPE_NORMAL,NULL,"Thank you for your answers!");
				}
			}
		}
		else
		{
			tp_server_send_message_str(tox,friend_number,TOX_MESSAGE_TYPE_NORMAL,NULL,"Waiting for other players ...");
		}
	}
}
