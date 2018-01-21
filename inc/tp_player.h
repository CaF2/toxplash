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
	guint32 player_id;
	//gchar *player_name;
	GArray *qa; ///< GStrings
	gint curr_qa;
	gint points;
}TPPlayer;

#include "gen/tp_player.h"
