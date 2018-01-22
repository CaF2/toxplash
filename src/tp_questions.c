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

#include "tp_player.h"
#include "tp_game.h"
#include "tp_questions.h"


static gchar *LOCAL_QUESTIONS[] =
{
	"What was your favorite dog?",
	"What is your favorite color",
	"When will you die?",
	"When did you get your first blurr?",
	"What does the fish say?",
	"How many cows exists on this planet?",
	"Who invented the roller coaster?",
	"Who is married with santa claus?",
	"Who will win ww3 for cats?",
	"If you would meet a bear, what would you say?",
	"What would you call your uncle from antarctica?",
	"What is \"i love you <3\" in Urgurkish?"
};

#define LOCAL_QUESTIONS_LENGTH (sizeof(LOCAL_QUESTIONS)/sizeof(LOCAL_QUESTIONS[0]))

TPQa *tp_qa_init(TPQa *self, gint question, gchar *answer)
{
	memset(self,'\0',sizeof(TPQa));

	self->question=question;
	
	if(answer)
		self->answer=g_strdup(answer);

	return self;
}

TPQa *tp_qa_new(gint question, gchar *answer)
{
	return tp_qa_init(malloc(sizeof(TPQa)),question,answer);
}

gchar *tp_qa_get_question(gint question)
{
	return LOCAL_QUESTIONS[question];
}

/**
	1 player = 2 total questions
	2 players = 2 total questions
	3 players = 3 total questions
	4 players = 4


*/
gboolean tp_generate_questions(TPGameRoom *self,gint num_questions)
{
	gint num_players=self->players->len;
	gint total_questions;
	
	TPPlayer *players=(TPPlayer*)self->players->data;
	
	if(num_players<=1)
	{
		total_questions=2;
	}
	else
	{
		total_questions=num_players;
	}
	
	for(gint rounds=0;rounds<num_questions;rounds++)
	{
		gint rand_val=rand();
	
		gint rand_pos=rand_val % LOCAL_QUESTIONS_LENGTH;
	
		printf("ROUND %d:: %d %d\n",rounds,rand_val,rand_pos);
	
		for(gint i=0;i<2;i++)
		{
			gint second_randomizer;
			
			/*
				12
				21
				
				12
				23
				31
			*/
			
			gint curr_rand_pos=rand_pos+i;
			
			if(curr_rand_pos>=LOCAL_QUESTIONS_LENGTH)
			{
				curr_rand_pos=0;
			}
	
			for(gint player_i=0;player_i<num_players;player_i++)
			{
				TPPlayer *curr_player=&players[player_i];
				
				printf("ptr:: %p\n",curr_player->qa);
				
				if(curr_player->qa==NULL)
				{
					curr_player->qa=g_array_new(FALSE,FALSE,sizeof(TPQa));
				}
				
				printf("ptr:: %p\n",curr_player->qa);
		
				TPQa input;
				
				tp_qa_init(&input,curr_rand_pos,NULL);
				
				g_array_append_val(curr_player->qa,input);
		
				TPQa *curr_qa=(TPQa*)&(curr_player->qa->data[curr_player->qa->len-1]);
			
				printf("sent question to %d, question: ID=%d STR=%s QAs=%d\n",curr_player->player_id,curr_qa->question,tp_qa_get_question(curr_qa->question),curr_player->qa->len);
				
				/*
					(0) max,
					(1) 0
					(0) 0
					(1) 1
					
					for this problem, curr_rand_pos belongs to a ring,
					so it requires raw counter to work. (rand_pos+i+player_i) simulates that. 
				*/
				if(rand_pos+i+player_i-rand_pos+1>=total_questions)
				{
					curr_rand_pos=rand_pos;
				}
				else if((curr_rand_pos+1)>=LOCAL_QUESTIONS_LENGTH)
				{
					curr_rand_pos=0;
				}
				else
				{
					curr_rand_pos++;
				}
			}
		
		
		}
	}
	
	return TRUE;
}
