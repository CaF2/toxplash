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

#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>

#include <glib.h>

typedef struct TPPlayer
{
	uint32_t player_id;
	//gchar *player_name;
	GArray *qa; ///< GStrings
	int curr_qa;
	double points;
	
	uint8_t voted; ///<if the player voted this round
}TPPlayer;

#include "tp_questions.h"

#include "gen/tp_player.h"
