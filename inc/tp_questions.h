#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>

#include <glib.h>

#include "tp_game.h"

typedef struct TPQa
{
	gint question;
	gchar *answer;
	gdouble points;
}TPQa;

#include "gen/tp_questions.h"
