// Super Grayland: Copyright 2021 Vincent Robinson under the MIT license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#pragma once

#include "common.h"

#include "map.h"
#include "screen.h"

// GME Namespace: Main game data
/*
	All the game data is held in the GME namespace. All data is ultimately held in the GME_Game
	superstruct. The game can be in multiple states, such as the main menu, a level, or the
	level editor. Some data is common to all states, such as the screen. Other things, like the
	map for a level, are held in specific state structs in a union. Each state struct has an
	associated enum value in GME_State and three associated functions:

	* GME_<STATE>_init(GME_Game *game, ...): Initializes the state. These can take more
	  parameters if they need more information to start up. These will automatically
	  deinitialize the current state with GME_deInitState. If there is a problem in
	  initialization, they will deinitialize themselves and throw an error.
	* GME_<STATE>_deInit(GME_Game *game): Deinitializes the state. These must not be called
	  unless the state they are deinitializing is currently active. It generally isn't
	  necessary to call these since the init functions will deinitialize automatically.
	* GME_<STATE>_loop(GME_Game *game): The mainloop for the state. These not be called unless
	  the state they are deinitializing is currently active. This will be called automatically
	  in GME_loop.

	In general, it is only necessary for most code to call GME_<state>_init, and everything else
	will be handled by the game automatically.
*/

// Defines the current state the game is in and the currently active part of the GME_Game union.
enum GME_State
{
	GME_State_NONE, // No state, i.e. the game will exit after the end of the mainloop.
	GME_State_MAIN_MENU,
	GME_State_LEVEL,
	GME_State_EDITOR
};

struct GME_Game;

// A struct containing all the data relevant to the main menu.
struct GME_MainMenu
{
};

#define GME_MM_init(game)
#define GME_MM_deInit(game)
#define GME_MM_loop(game)
// void GME_MM_init(struct GME_Game *game);
// void GME_MM_deInit(struct GME_Game *game);
// void GME_MM_loop(struct GME_Game *game);

// A struct containing all the data relevant to a level that is being played in.
struct GME_Level
{
	// The static map data
	struct MAP_Map Map;
	// The dynamically sorted list of objects
	// struct SOL_List Objs;
};

void GME_LVL_init(struct GME_Game *game);
void GME_LVL_deInit(struct GME_Game *game);
void GME_LVL_loop(struct GME_Game *game);

// A struct containing all the data relevant to the level editor.
struct GME_Editor
{
	// The static map data
	struct MAP_Map Map;
	// The dynamically sorted list of map objects
	// struct SLL(MAP_Obj)_List Objs;
};

#define GME_EDT_init(game)
#define GME_EDT_deInit(game)
#define GME_EDT_loop(game)
// void GME_EDT_init(struct GME_Game *game);
// void GME_EDT_deInit(struct GME_Game *game);
// void GME_EDT_loop(struct GME_Game *game);

// This struct is EVERYTHING in the game (except for a very few scattered global variables).
// Everything in this struct will be zeroed when the program is started.
struct GME_Game
{
	// The current game state.
	enum GME_State State;
	// All possible state data. The active member is defined by `State`.
	union {
		struct GME_MainMenu MainMenu;
		struct GME_Level Level;
		struct GME_Editor Editor;
	};
	// The screen information, common to all states.
	struct SCR_Screen Screen;
};

// Initializes the shared components of the game, but none of the specific states, which must
// be initialized separately. Throws an error if it could not be initialized. Must always have
// a corresponding GME_deInit.
void GME_init(struct GME_Game *game);
// Deinitializes the entire game, states and shared data alike.
void GME_deInit(struct GME_Game *game);
// Deinitializes the current state. If the state is GME_State_NONE, nothing happens.
void GME_deInitState(struct GME_Game *game);

// The mainloop for the entire game.
void GME_loop(struct GME_Game *game);
