#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>

#include <glib.h>

#include <sodium/utils.h>
#include <tox/tox.h>

#include "tp_player.h"

typedef enum TPGameRoomStates
{
	TP_GAME_ROOM_STATE_NONE,
	TP_GAME_ROOM_STATE_WAIT_PLAYERS,
	TP_GAME_ROOM_STATE_PLAYING,
	TP_GAME_ROOM_STATE_WAIT_LEAVE,
	TP_GAME_ROOM_LENGTH
}TPGameRoomStates;

typedef struct TPGameRoom
{
	gchar *room_name;
	TPGameRoomStates state;
	gint round;
	GArray *players; ///< TPPlayer
	GArray *taken_questions;
}TPGameRoom;

#include "gen/tp_game.h"
#include "tp_questions.h"
#include "gen/main.h"
