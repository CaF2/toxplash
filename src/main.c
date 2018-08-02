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

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
 
#include <unistd.h>
 
#include <sodium/utils.h>
#include <tox/tox.h>
#include <glib.h>

#include "tp_game.h"
#include "tp_questions.h"
#include "main.h"

typedef struct DHT_node
{
	const char *ip;
	uint16_t port;
	const char key_hex[TOX_PUBLIC_KEY_SIZE*2 + 1];
	unsigned char key_bin[TOX_PUBLIC_KEY_SIZE];
} DHT_node;

int GLOBAL_TEST=FALSE;

const char *savedata_filename = "savedata.tox";
const char *savedata_tmp_filename = "savedata.tox.tmp";

uint32_t tp_server_send_message(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, glong length, TOX_ERR_FRIEND_SEND_MESSAGE *error)
{
	uint32_t res;
	gchar *debugmsg;
	
	if(length<0)
	{
		length=strlen(message);
	}

	debugmsg=g_strndup((gchar*)message,length);

	if(!GLOBAL_TEST)
	{
		res=tox_friend_send_message(tox, friend_number, type, message, length, error);
		
		printf("SENT [%d]: %s\n",friend_number,debugmsg);
	}
	else
	{
		res=0;
		printf("SENT [%d]: %s\n",friend_number,debugmsg);
	}
	
	g_free(debugmsg);
	
	return res;
}

uint32_t tp_server_send_message_str(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, TOX_ERR_FRIEND_SEND_MESSAGE *error, const gchar *message,...)
{
	va_list args;
	g_autofree char *send_str=NULL;

	va_start(args,message);
	vasprintf(&send_str, message, args);

	uint32_t retval=tp_server_send_message(tox,friend_number,type,(uint8_t*)message,strlen(message),error);
	
	return retval;
}

uint32_t tp_server_room_send_message(Tox *tox, TPGameRoom *room, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, TOX_ERR_FRIEND_SEND_MESSAGE *error)
{
	uint32_t result=0;

	TPPlayer *players=(TPPlayer*)room->players->data;
	for(gint ii=0;ii<room->players->len;ii++)
	{
		guint32 curr_player=players[ii].player_id;
	
		uint32_t curr_result=tp_server_send_message(tox,curr_player,type,message,length,error);
		
		result|=curr_result;
	}
	
	return result;
}

uint32_t tp_server_room_send_message_str(Tox *tox, TPGameRoom *room, TOX_MESSAGE_TYPE type, TOX_ERR_FRIEND_SEND_MESSAGE *error, const gchar *message,...)
{
	va_list args;
	g_autofree char *send_str=NULL;

	va_start(args,message);
	vasprintf(&send_str, message, args);

	uint32_t retval=tp_server_room_send_message(tox,room,type,(uint8_t*)send_str,-1,error);
	 
	return retval;
}

Tox *create_tox()
{
	Tox *tox;
 
	struct Tox_Options options;
 
	tox_options_default(&options);
 
	FILE *f = fopen(savedata_filename, "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);
 
		guint8 *savedata = malloc(fsize);
 
		fread(savedata, fsize, 1, f);
		fclose(f);
 
		options.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
		options.savedata_data = savedata;
		options.savedata_length = fsize;
 
		tox = tox_new(&options, NULL);
 
		free(savedata);
	}
	else
	{
		tox = tox_new(&options, NULL);
	}
 
	return tox;
}
 
void update_savedata_file(const Tox *tox)
{
	size_t size = tox_get_savedata_size(tox);
	guint8 *savedata = malloc(size);
	tox_get_savedata(tox, savedata);
 
	FILE *f = fopen(savedata_tmp_filename, "wb");
	fwrite(savedata, size, 1, f);
	fclose(f);
 
	rename(savedata_tmp_filename, savedata_filename);
 
	free(savedata);
}
 
void bootstrap(Tox *tox)
{
	DHT_node nodes[] =
	{
		{"178.62.250.138",			 33445, "788236D34978D1D5BD822F0A5BEBD2C53C64CC31CD3149350EE27D4D9A2F9B6B",{0}},
		{"2a03:b0c0:2:d0::16:1",	   33445, "788236D34978D1D5BD822F0A5BEBD2C53C64CC31CD3149350EE27D4D9A2F9B6B",{0}},
		{"tox.zodiaclabs.org",		 33445, "A09162D68618E742FFBCA1C2C70385E6679604B2D80EA6E84AD0996A1AC8A074",{0}},
		{"163.172.136.118",			33445, "2C289F9F37C20D09DA83565588BF496FAB3764853FA38141817A72E3F18ACA0B",{0}},
		{"2001:bc8:4400:2100::1c:50f", 33445, "2C289F9F37C20D09DA83565588BF496FAB3764853FA38141817A72E3F18ACA0B",{0}},
		{"128.199.199.197",			33445, "B05C8869DBB4EDDD308F43C1A974A20A725A36EACCA123862FDE9945BF9D3E09",{0}},
		{"2400:6180:0:d0::17a:a001",   33445, "B05C8869DBB4EDDD308F43C1A974A20A725A36EACCA123862FDE9945BF9D3E09",{0}},
		{"biribiri.org",			   33445, "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67",{0}}
	};
 
	for (size_t i = 0; i < sizeof(nodes)/sizeof(DHT_node); i ++)
	{
		sodium_hex2bin(nodes[i].key_bin, sizeof(nodes[i].key_bin),
					   nodes[i].key_hex, sizeof(nodes[i].key_hex)-1, NULL, NULL, NULL);
		tox_bootstrap(tox, nodes[i].ip, nodes[i].port, nodes[i].key_bin, NULL);
	}
}
 
void print_tox_id(Tox *tox)
{
	uint8_t tox_id_bin[TOX_ADDRESS_SIZE];
	tox_self_get_address(tox, tox_id_bin);
 
	char tox_id_hex[TOX_ADDRESS_SIZE*2 + 1];
	sodium_bin2hex(tox_id_hex, sizeof(tox_id_hex), tox_id_bin, sizeof(tox_id_bin));
 
	for (size_t i = 0; i < sizeof(tox_id_hex)-1; i ++)
	{
		tox_id_hex[i] = toupper(tox_id_hex[i]);
	}
 
	printf("Tox ID: %s\n", tox_id_hex);
}
 
void friend_request_cb(Tox *tox, const uint8_t *public_key, const uint8_t *message, size_t length, void *user_data)
{
	uint32_t friend_number=tox_friend_add_norequest(tox, public_key, NULL);
	
	update_savedata_file(tox);
	
	tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, "Hello and welcome to Toxplash (TM)!");
}
 
//random integer from 0 to n-1
static int irand(int n)
{
	int r, rand_max = RAND_MAX - (RAND_MAX % n);
	/* reroll until r falls in a range that can be evenly
	 * distributed in n bins.  Unless n is comparable to
	 * to RAND_MAX, it's not *that* important really. */
	while ((r = rand()) >= rand_max);
	return r / (rand_max / n);
}

void shuffle(char **list, size_t len)
{
	int j;
	char *tmp;
	while(len)
	{
		j = irand(len);
		if (j != len - 1)
		{
			tmp = list[j];
			list[j] = list[len - 1];
			list[len - 1] = tmp;
		}
		len--;
	}
}
 
void friend_message_cb(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *user_data)
{
	GString *mod_message=g_string_new_len((gchar*)message, length);
	
	gchar *handleable_str=g_utf8_strdown(mod_message->str,mod_message->len);
	
	TPGameRoom *room=tp_game_room_find_from_user(friend_number);
	
	if(room==NULL)
	{
		if(strcmp(handleable_str,"create")==0)
		{
			GDateTime *time=g_date_time_new_now_local();
	
			g_autofree gchar *time_str=g_date_time_format(time,"%Y%m%d%H%M%S");
		
			g_date_time_unref(time);
		
			tp_game_room_add_user_from_room_name(time_str,friend_number);
		
			g_autofree gchar *output_str=g_strdup_printf("Added you to room %s",time_str);
		
			tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, output_str);
		}
		else if(strcmp(handleable_str,"help")==0)
		{
			const char *helpstr="List of available commands:\n"
			"<room number>    -- Join a room\n"
			"create           -- Create a new room\n"
			"help             -- Display this text\n";
		
			tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, helpstr);
		}
		else
		{
			TPGameRoom *search_room=tp_game_room_find_from_name(handleable_str);
		
			if(search_room)
			{
				tp_game_room_add_user(search_room,friend_number);
			}
			else
			{
				tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, "Sorry sir, that is not a valid room name. You may create a room by typing \"create\"!");
			}
		}
	}
	else
	{
		switch(room->state)
		{
			case TP_GAME_ROOM_STATE_WAIT_PLAYERS:
			if(strcmp(handleable_str,"start")==0)
			{
				tp_game_play(tox,room);
			}
			else if(strcmp(handleable_str,"help")==0)
			{
				const char *helpstr="List of available commands:\n"
				"start            -- Start the toxsplash game\n"
				"rand t1 t2 t3 tN -- Send one of the text strings to all in the room\n"
				"rrand min max    -- Generate a random number between range\n"
				"shuffle t1 t2 tN -- Put the selected items in a randomized order\n"
				"avalon           -- Shuffle the avalon cards (play it instead)\n"
				"list             -- List users\n"
				"exit             -- Quit toxsplash\n"
				"help             -- Display this text\n";
			
				tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, helpstr);
			}
			else if(strcmp(handleable_str,"avalon")==0)
			{
				//merlin knows evil
				//percival knows merlin
				
				//morgana reveal as merlin
				//does not reveal to evil
				//mordred does not reveal to merlin
				//
				char *cards[]={
				"MERLIN","PERCIVAL","Loyal servant of Arthur","Loyal servant of Arthur","Loyal servant of Arthur","Loyal servant of Arthur","Loyal servant of Arthur",
				"MORDRED'S ASSASIN","MORGANA","OBERON","MORDRED","Minion of mordred","Minion of mordred","Minion of mordred"
				};
				
				size_t num_cards=sizeof(cards)/sizeof(cards[0]);
				
				g_autofree char **random_card_deck=malloc(num_cards*sizeof(char*));
				
				for(size_t i=0;i<num_cards;i++)
				{
					random_card_deck[i]=g_strdup(cards[i]);
				}
				
				shuffle(random_card_deck,num_cards);
				
				for(size_t i=0;i<num_cards;i++)
				{
					printf(" %s ",random_card_deck[i]);
				}
				
				printf("\n");
				
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"Starting the game!");
				
				TPPlayer *players=(TPPlayer*)room->players->data;
				for(gint i=0;i<room->players->len;i++)
				{
					guint32 itr_player=players[i].player_id;
				
					g_autoptr(GString) output=g_string_sized_new(100);
				
					g_string_append_printf(output,"You recieved the card: %s",random_card_deck[i]);
	
					tp_server_send_message(tox,itr_player,TOX_MESSAGE_TYPE_NORMAL,output->str,output->len,NULL);
				}
			}
			else if(g_str_has_prefix(handleable_str,"rand"))
			{
				g_autoptr(GStrArr) functvars=g_strsplit(handleable_str," ",-1);
				size_t num_args=g_strv_length(functvars);
				
				if(num_args<3)
				{
					tp_server_send_message(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, "rand requires more arguments",-1, NULL);
					break;
				}
				GStrArr *cards=&functvars[1];
				
				size_t num_cards=num_args-1;
				
				g_autofree char **random_card_deck=malloc(num_cards*sizeof(char*));
				
				for(size_t i=0;i<num_cards;i++)
				{
					random_card_deck[i]=g_strdup(cards[i]);
				}
				
				shuffle(random_card_deck,num_cards);
				
				for(size_t i=0;i<num_cards;i++)
				{
					printf(" %s ",random_card_deck[i]);
				}
				
				printf("\n");
				
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"> %s",handleable_str);
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"> %s",random_card_deck[0]);
			}
			else if(g_str_has_prefix(handleable_str,"rrand"))
			{
				g_autoptr(GStrArr) functvars=g_strsplit(handleable_str," ",-1);
				size_t num_args=g_strv_length(functvars);
				
				if(num_args!=3)
				{
					tp_server_send_message(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, "format is \"rrand min max\"",-1, NULL);
					break;
				}
				
				int min=atoi(functvars[1]);
				int max=atoi(functvars[2]);
				
				if(min>max)
				{
					int tmp=min;
					min=max;
					max=tmp;
				}
				
				int range=max-min+1;
				
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"> %s",handleable_str);
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"> %d",min+irand(range));
			}
			else if(g_str_has_prefix(handleable_str,"shuffle"))
			{
				g_autoptr(GStrArr) functvars=g_strsplit(handleable_str," ",-1);
				size_t num_args=g_strv_length(functvars);
				
				if(num_args<3)
				{
					tp_server_send_message(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, "shuffle requires more arguments!",-1, NULL);
					break;
				}
				GStrArr *cards=&functvars[1];
				
				size_t num_cards=num_args-1;
				
				g_autofree char **random_card_deck=malloc(num_cards*sizeof(char*));
				
				for(size_t i=0;i<num_cards;i++)
				{
					random_card_deck[i]=g_strdup(cards[i]);
				}
				
				shuffle(random_card_deck,num_cards);
				
				g_autoptr(GString) output=g_string_sized_new(100);
				
				g_string_append(output,">");
				
				for(gint i=0;i<num_cards;i++)
				{
					g_string_append_printf(output," %s",random_card_deck[i]);
				}
				
				tp_server_room_send_message_str(tox, room, TOX_MESSAGE_TYPE_NORMAL,NULL,"> %s",handleable_str);
				tp_server_room_send_message(tox,room,TOX_MESSAGE_TYPE_NORMAL,output->str,output->len,NULL);
			}
			else if(strcmp(handleable_str,"list")==0)
			{
				g_autofree gchar *user_list=g_string_free(tp_game_room_dump_players(room),FALSE);
		
				tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, user_list);
			}
			else if(strcmp(handleable_str,"exit")==0)
			{
				tp_game_room_remove_user(room,friend_number);

			}
			else
			{
				tp_server_send_message_str(tox, friend_number, TOX_MESSAGE_TYPE_NORMAL, NULL, "If you send me \"start\" I will start a game for you!");
			}
			break;
			case TP_GAME_ROOM_STATE_PLAYING:
			{
				//if(room->round)
				
				tp_game_answer(room, tox, friend_number,mod_message->str,mod_message->len);
				
				g_string_prepend(mod_message,"MES:");
				g_string_prepend(mod_message,room->room_name);
				
				tp_server_send_message(tox, friend_number, type, (uint8_t*)mod_message->str, mod_message->len, NULL);
				
				break;
			}
			default:
			break;
		}
	}
	
	g_string_free(mod_message,TRUE);
	
	g_free(handleable_str);
}
 
void self_connection_status_cb(Tox *tox, TOX_CONNECTION connection_status, void *user_data)
{
	switch (connection_status)
	{
		case TOX_CONNECTION_NONE:
			printf("Offline\n");
			break;
		case TOX_CONNECTION_TCP:
			printf("Online, using TCP\n");
			break;
		case TOX_CONNECTION_UDP:
			printf("Online, using UDP\n");
			break;
	}
}

int do_toxsplash_server()
{
	Tox *tox = create_tox();
 
	const gchar *name = "Toxplash!";
	tox_self_set_name(tox, (guint8*)name, strlen(name), NULL);
 
	const char *status_message = "Answer questions like never before!";
	tox_self_set_status_message(tox, (guint8*)status_message, strlen(status_message), NULL);
 
	bootstrap(tox);
 
	print_tox_id(tox);
 
	tox_callback_friend_request(tox, friend_request_cb);
	tox_callback_friend_message(tox, friend_message_cb);
 
	tox_callback_self_connection_status(tox, self_connection_status_cb);
 
	update_savedata_file(tox);
 
	while (1)
	{
		tox_iterate(tox, NULL);
		usleep(tox_iteration_interval(tox) * 1000);
	}
 
	tox_kill(tox);
}

int do_test()
{
	char *input;
	
	gchar *msg="create";
	friend_message_cb(NULL,0,TOX_MESSAGE_TYPE_NORMAL,(uint8_t*)msg,strlen(msg),NULL);

	while(1)
	{
		scanf("%m[^\n]%*c",&input);
		
		printf("GOT IN: %s\n",input);
		
		if(input)
			friend_message_cb(NULL,1,TOX_MESSAGE_TYPE_NORMAL,(uint8_t*)input,strlen(input),NULL);
		
		free(input);
	}
	
	return 0;
}

int main(int argv, char **argc)
{
/*	srand(time(NULL));*/

/*	GArray *arr=g_array_sized_new(FALSE,FALSE,sizeof(int),10);*/

/*	for(int i=0;i<10;i++)*/
/*	{*/
/*		g_array_append_val(arr,i);*/
/*	}*/
/*	*/
/*	for(int i=0;i<arr->len;i++)*/
/*	{*/
/*		int val=g_array_index(arr,int,i);*/
/*		*/
/*		printf("%d,",val);*/
/*	}*/
/*	*/
/*	printf("\n");*/
/*	*/
/*	GArray *sarr=shuffle_array(arr);*/
/*	*/
/*	for(int i=0;i<sarr->len;i++)*/
/*	{*/
/*		int val=g_array_index(sarr,int,i);*/
/*		*/
/*		printf("%d,",val);*/
/*	}*/
/*	*/
/*	printf("\n");*/
/*	*/
/*	return 0;*/
	//////////////////
	
	int test=0;

	srand(time(NULL));

	if(argv>1)
	{
		test=(strcmp(argc[1],"test")==0);
		
		printf("IN %s %d\n",argc[1],strcmp(argc[1],"test"));
	}

	if(!(test))
	{
		return do_toxsplash_server();
	}
	else
	{
		GLOBAL_TEST=TRUE;
		return do_test();
	}
}


