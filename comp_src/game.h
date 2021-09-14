// Super Grayland, Copyright 2020 Vincent Robinson under the zlib license. See 'LICENSE.txt' for more information.

#pragma once

#include "common.h"

// What state the game is currently in
enum state
{
	STATE_QUIT,
	STATE_STARTUP,
	STATE_LEVEL
};

// The game and all data common to it that is not tied to any other specific thing
struct game
{
	// Current game state
	enum state state;
};

// Global game variable
extern struct game g_game;

// Initialize/deinitialize all game-related stuff
bool init_game(void);
void deinit_game(void);

// Game main loop, runs everything for a single frame
bool mainloop(void);
