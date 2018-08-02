/**
Toxplash
Copyright (C) 2018 Florian Evaldsson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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

int tp_game_room_reset_votes(TPGameRoom *self)
{
	GArray *players=self->players;
	for(gint i=0;i<players->len;i++)
	{
		TPPlayer *player=&g_array_index(players,TPPlayer,i);
		
		player->voted=0;
	}
	
	return 0;
}

/**
	two rounds a la 60 sec, 2 questions (both can be answered at the same time).
	In the end one question all will answer
	
	-vad som inte funkar::
	*Man kan rösta efter röstfasen,
	*Under röstfasen så visas bara en röst
	
	** Göra en questionarray
	{antal spelare}[q1,q2,q3]
	Shuffle och sprid till olika spelare,
	Gör det sedan igen på andra frågan, och granska att de inte är samma som innan (shuffla om)
	
	Under räkningsfasen shuffla frågorna igen och börja att be om svar för frågorna.
*/
void *tp_game_play_thread(void* argument)
{
	gchar *str=NULL;

	void **input=argument;

	TPGameRoom *self=input[0];
	Tox *tox=input[1];
	
	free(input);
	
	GTimer *timer=g_timer_new();
	
	GArray *players_arr=self->players;
	
	TPPlayer *players=(TPPlayer*)players_arr->data;
	
	for(gint ii=0;ii<players_arr->len;ii++)
	{
		TPPlayer *player=&g_array_index(players_arr,TPPlayer,ii);
	
		guint32 curr_player_id=player->player_id;
		
		TPQa *qa=&g_array_index(player->qa,TPQa,0);
		
		str=g_strdup_printf("==%s==",tp_qa_get_question(qa->question));
	
		tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
	
		g_free(str);
	}
	
	g_timer_start(timer);
	
	int total_rounds=self->std_rounds;
	
	for(self->round=0;self->round<total_rounds;self->round++)
	{
		//question phase
		tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,"Question phase, answer your questions!");
		
		self->is_voting=0;
		
		for(gint elapsed=20;elapsed>=0;elapsed--)
		{
			TPPlayer *players=(TPPlayer*)players_arr->data;
			for(gint ii=0;ii<players_arr->len;ii++)
			{
				TPPlayer *curr_player=&players[ii];
			
				guint32 curr_player_id=curr_player->player_id;
				
				TPQa *qa=(TPQa*)curr_player->qa->data;
				
				//if not answering
				if(curr_player->curr_qa<self->round*self->num_questions)
				{
					curr_player->curr_qa=self->round*self->num_questions;
				}
				
				if(curr_player->curr_qa<(self->round+1)*self->num_questions)
				{
					str=g_strdup_printf("== %d ==",elapsed);
					
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
					
					g_free(str);
					
					str=g_strdup_printf("== %s %d ==",tp_qa_get_question(qa[curr_player->curr_qa].question),players[ii].qa->len);
					
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
					
					g_free(str);
				}
				else
				{
					str=g_strdup_printf("== Waiting for other players. %d sec left ... ==",elapsed);
	
					tp_server_send_message_str(tox,curr_player_id,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
	
					g_free(str);
				}
			}
		
			g_usleep(1000000);
		}
		
		self->is_voting=1;
		
		g_usleep(1000000);
		
		tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,"Voting phase, get ready to vote!");
		
		g_usleep(1000000);
		
		for(int question=0;question<self->num_questions;question++)
		{
			TPVote *vote=&g_array_index(self->votes,TPVote,question);
			
			//give a win to a question if there is only one answer
			TPQa *lonely_qa=NULL;
			TPPlayer *lonely_player=NULL;
			int answers=0;
			
			GArray *pplayers=self->players;
			for(gint ii=0;ii<pplayers->len;ii++)
			{
				TPPlayer *curr_player=&g_array_index(pplayers,TPPlayer,ii);
		
				guint32 curr_player_id=curr_player->player_id;
				
				int index=0;
				
				TPQa *qa=tp_player_get_question_from_id(curr_player,vote->question,&index);
				
				if(qa->answer)
				{
					if(answers==0)
					{
						answers++;
						lonely_qa=qa;
						lonely_player=curr_player;
					}
					else
					{
						break;
					}
				}
			}
			
			if(answers==1)
			{
				//lonely_player->points+=2;
				lonely_qa->votes=2;
			}
			
			//print alternatives to players
			static int max_elapsed=10;
			
			for(int elapsed=max_elapsed;elapsed>=0 && answers>1;elapsed--)
			{
				str=g_strdup_printf("== %d ==",elapsed);
	
				tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
		
				g_free(str);
				
				//if(elapsed==max_elapsed)
				{
					str=g_strdup_printf("== %s ==",tp_qa_get_question(vote->question));
			
					tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
			
					g_free(str);
				}
				
				//all players can vote
				GArray *players=self->players;
				for(gint ii=0;ii<players->len;ii++)
				{
					TPPlayer *curr_player=&g_array_index(players,TPPlayer,ii);
			
					guint32 curr_player_id=curr_player->player_id;
					
					int index=0;
					
					TPQa *qa=tp_player_get_question_from_id(curr_player,vote->question,&index);
					
					str=g_strdup_printf("%c) %s",'A'+ii,qa->answer);
					
					tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
					
					g_free(str);
				}
				
				g_usleep(1000000);
			}
			
			tp_game_room_reset_votes(self);
			
			//count votes (all players)
			int next_most=-1;
			int most=-1;
			TPPlayer *win_player=NULL;
			TPQa *win_qa=NULL;
			int win_player_index=-1;
			
			GPtrArray *players=vote->players;
			for(gint ii=0;ii<players->len;ii++)
			{
				TPPlayer *curr_player=g_ptr_array_index(players,ii);
				
				int index=0;
				
				TPQa *qa=tp_player_get_question_from_id(curr_player,vote->question,&index);
				
				curr_player->points+=qa->votes;
				
				if(qa->votes>most)
				{
					next_most=most;
					most=qa->votes;
					win_player=curr_player;
					win_player_index=ii;
					win_qa=qa;
				}
			}
			
			guint32 win_player_id=win_player->player_id;
			
			if(next_most<=0)
			{
				str=g_strdup_printf("%c) ((VOTES=%d,,%d)) [%s] %d TOXSPLASH!",'A'+win_player_index,most,next_most,win_qa->answer,win_player_id);
			}
			else
			{
				str=g_strdup_printf("%c) ((VOTES=%d,,%d)) [%s] %d WON!",'A'+win_player_index,most,next_most,win_qa->answer,win_player_id);
			}
					
			tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,str);
			
			g_free(str);
			
			g_usleep(1000000*5);
		}
		
		//TODO clean up votes (reset)
	}
	
	tp_server_room_send_message_str(tox,self,TOX_MESSAGE_TYPE_NORMAL,NULL,"Times up!");
	
	return NULL;
}

int tp_game_play(Tox *tox,TPGameRoom *room)
{
	if(!tp_generate_questions(room,2,2))
	{
		printf("ERROR: Something went wrong with generating the questions!\n");
	}
	
	room->state=TP_GAME_ROOM_STATE_PLAYING;
	
	pthread_t game;
	
	void **input=malloc(2*sizeof(void*));
	
	input[0]=room;
	input[1]=tox;

	tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"Starting the game!");

	int result_code=pthread_create(&game, NULL, tp_game_play_thread, input);
	
	return result_code;
}

void tp_game_answer(TPGameRoom *self, Tox *tox, uint32_t friend_number, const uint8_t *message, size_t length)
{
	TPPlayer *player=tp_game_room_find_user(self,friend_number);
	
	if(player)
	{
		TPQa *qas=(TPQa*)player->qa->data;
		
		TPQa *qa=&qas[player->curr_qa];
		
		if(self->is_voting==0)
		{
			if(!qa->answer)
			{
				qa->answer=strndup(message,length);
				
				player->curr_qa++;
				
				if(player->curr_qa>=(self->round+1)*self->num_questions)
				{
					tp_server_send_message_str(tox,friend_number,TOX_MESSAGE_TYPE_NORMAL,NULL,"Thank you for your answers!");
				}
			}
		}
		else if(self->is_voting==1 && player->voted==0)
		{
			int tplayer=-1;
			
			if(message[0]>='A' && message[0]<='Z')
			{
				tplayer=message[0]-'A';
			}
			else if(message[0]>='a' && message[0]<='z')
			{
				tplayer=message[0]-'a';
			}
			
			TPVote *vote=&g_array_index(self->votes,TPVote,self->round);
				
			GPtrArray *players=vote->players;
			
			if(tplayer>=0 && tplayer<players->len)
			{
				TPPlayer *curr_player=g_ptr_array_index(players,tplayer);
				
				int index=0;
				
				TPQa *qa=tp_player_get_question_from_id(curr_player,vote->question,&index);
				
				qa->votes++;
				player->voted=1;
			}
			else
			{
				printf("ERROR on getting player");
			}
		}
		else
		{
			tp_server_send_message_str(tox,friend_number,TOX_MESSAGE_TYPE_NORMAL,NULL,"Waiting for other players ...");
		}
	}
}
