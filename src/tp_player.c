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

TPPlayer *tp_player_init(TPPlayer *self, guint32 player_id)
{
	memset(self,'\0',sizeof(TPPlayer));

	self->player_id=player_id;
	//self->qa=g_array_new(FALSE,FALSE,sizeof(TPQa));

	printf("pointer made %p\n",self->qa);

	return self;
}

TPQa *tp_player_get_question_from_id(TPPlayer *self, intptr_t question, int *index)
{
	GArray *qa=self->qa;
	
	if(index)
	{
		*index=-1;
	}
	
	for(int i=0;i<qa->len;i++)
	{
		TPQa *cur_qa=&g_array_index(qa,TPQa,i);
		
		if(cur_qa->question==question)
		{
			if(index)
			{
				*index=i;
			}
			return cur_qa;
		}
	}
	
	return NULL;
}
