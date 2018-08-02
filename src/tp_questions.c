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

//////////////////


TPQa *tp_qa_init(TPQa *self, gint question, TPPlayer *parent)
{
	memset(self,'\0',sizeof(TPQa));

	self->question=question;
	
	self->parent=parent;

	return self;
}

TPQa *tp_qa_new(gint question, TPPlayer *parent)
{
	return tp_qa_init(malloc(sizeof(TPQa)),question,parent);
}

TPVote *tp_vote_init(TPVote *self, gint question)
{
	memset(self,'\0',sizeof(TPVote));

	self->question=question;

	return self;
}

TPVote *tp_vote_new(gint question)
{
	return tp_vote_init(malloc(sizeof(TPVote)),question);
}

/////////////////

gchar *tp_qa_get_question(gint question)
{
	return LOCAL_QUESTIONS[question];
}

static int irand(int n)
{
	int r, rand_max = RAND_MAX - (RAND_MAX % n);
	/* reroll until r falls in a range that can be evenly
	 * distributed in n bins.  Unless n is comparable to
	 * to RAND_MAX, it's not *that* important really. */
	while ((r = rand()) >= rand_max);
	return r / (rand_max / n);
}

GArray *clone_array(GArray *in)
{
	if(in==NULL)
	{
		return NULL;
	}

	printf("sizeof GArray:: %d %d\n",sizeof(GArray),malloc_usable_size(in));

	guint in_element_size=g_array_get_element_size(in);

	GArray *out_arr=g_array_sized_new(FALSE,FALSE,in_element_size,in->len);
	//out_arr->data=malloc(in_element_size*in->len);
	memcpy(out_arr->data,in->data,in_element_size*in->len);
	out_arr->len=in->len;
	
	return out_arr;
}

GArray *shuffle_array(GArray *in)
{
	if(in==NULL)
	{
		return NULL;
	}

	guint in_element_size=g_array_get_element_size(in);

	GArray *out_arr=clone_array(in);
	
	int j;
	char tmp[in_element_size];
	
	#define __out_arr_pos(pos) (((char*)out_arr->data)+((pos)*in_element_size))
	
	int len=in->len;
	
	while(len)
	{
		j = irand(len);
		if (j != len - 1)
		{
			memcpy(tmp,__out_arr_pos(j),in_element_size);
			memcpy(__out_arr_pos(j),__out_arr_pos(len - 1),in_element_size);
			memcpy(__out_arr_pos(len - 1),tmp,in_element_size);
		}
		len--;
	}
	
	return out_arr;
}

int add_player_to_vote(TPGameRoom *self,TPPlayer *player,gint question)
{
	GArray *votes=NULL;

	if(!self->votes)
	{
		self->votes=g_array_sized_new(FALSE,FALSE,sizeof(TPVote),10);
	}
	
	votes=self->votes;
	
	int found=0;
	
	for(int i=0;i<votes->len;i++)
	{
		TPVote *data=&g_array_index(votes,TPVote,i);
		
		if(data->question==question)
		{
			///TODO maybe a check if it does not exist in the array first
			g_ptr_array_add(data->players,player);
		}
	}
	
	if(found==0)
	{
		TPVote data;
		tp_vote_init(&data,question);
		
		data.players=g_ptr_array_sized_new(2);
		
		g_ptr_array_add(data.players,player);
		
		g_array_append_val(votes,data);
		
	}
}

/**
	1 player = 2 total questions
	2 players = 2 total questions
	3 players = 3 total questions {1,2},{2,3},{3,1}
	4 players = 4

	shuffle orig arr {1,2,3,4,5,6,7,8,9}
	feed take out (num of players) of the last questions.  {1,2,3,4,5,6,7,8,9} = saveq:{1,2,3,4,5,6,7} round1q:{8,9}
	repeat the process during each round
	
	likely num_questions=2 (2 questions per user), 2 std_rounds
*/
int tp_generate_questions(TPGameRoom *self,gint num_questions,gint std_rounds)
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
	
	self->num_questions=num_questions;
	self->std_rounds=std_rounds;
	
	for(gint rounds=0;rounds<(num_questions*std_rounds);rounds++)
	{
		gint rand_val=rand();
	
		gint rand_pos=rand_val % LOCAL_QUESTIONS_LENGTH;
	
		printf("ROUND %d:: %d %d\n",rounds,rand_val,rand_pos);
		
/*		GArray *user*/
		
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
				
				tp_qa_init(&input,curr_rand_pos,curr_player);
				add_player_to_vote(self,curr_player,curr_rand_pos);
				
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
