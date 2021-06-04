// Super Grayland: Copyright 2021 Vincent Robinson under the MIT license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#include "game.h"

char *ErrorInfo = NULL;

// In here so the warning is only issued once.
#ifdef DEBUG
#warning Debugging is enabled with a reserved register.
#endif

// Messages for each error code in GME_Error.
const char *ErrorMessages[] = {
	NULL,
	NULL,
	"Not enough memory for:",
	"File not found:",
};

void GME_LVL_init(struct GME_Game *game)
{
	struct GME_Level *level = &game->Level;
	struct MAP_Map *map = &level->Map;

	GME_deInitState(game);
	game->State = GME_State_LEVEL;

	MAP_init(map);

	SCR_TB_drawAllTiles(&game->Screen, map, 0, 0);
}

void GME_LVL_deInit(struct GME_Game *game)
{
	struct GME_Level *level = &game->Level;

	MAP_deInit(&level->Map);

	COM_zero(level);
}

void GME_LVL_loop(struct GME_Game *game)
{
	struct MAP_Map *map = &game->Level.Map;
	struct SCR_Screen *screen = &game->Screen;

	if (_keytest(RR_ESC)) {
		game->State = GME_State_NONE;
		return;
	}

	if (_keytest(RR_BCKSPC)) {
		SCR_scrollAbsolute(screen, map, 0, 0);
	} else {
		MAP_Scroll speed = 1;
		if (_keytest(RR_DIAMOND))
			speed = 8;

		MAP_Scroll shift_x = 0;
		MAP_Scroll shift_y = 0;

		if (_keytest(RR_RIGHT))
			shift_x += speed;
		if (_keytest(RR_LEFT))
			shift_x -= speed;

		if (_keytest(RR_DOWN))
			shift_y += speed;
		if (_keytest(RR_UP))
			shift_y -= speed;

		SCR_scroll(screen, map, shift_x, shift_y);
	}

	SCR_drawTileBuffer(screen,
			FXD_numer(MAP_Scroll, map->ScrollX), FXD_numer(MAP_Scroll, map->ScrollY));

	SCR_swap();
}

void GME_init(struct GME_Game *game)
{
	SCR_init(&game->Screen);

	GME_LVL_init(game);
}

void GME_deInit(struct GME_Game *game)
{
	GME_deInitState(game);
	SCR_deInit(&game->Screen);

	COM_zero(game);
}

void GME_deInitState(struct GME_Game *game)
{
	switch (game->State) {
	case GME_State_MAIN_MENU:
		GME_MM_deInit(game);
		break;
	case GME_State_LEVEL:
		GME_LVL_deInit(game);
		break;
	case GME_State_EDITOR:
		GME_EDT_deInit(game);
		break;
	default:
		break;
	}

	game->State = GME_State_NONE;
}

void GME_loop(struct GME_Game *game)
{
	switch (game->State) {
	case GME_State_MAIN_MENU:
		GME_MM_loop(game);
		break;
	case GME_State_LEVEL:
		GME_LVL_loop(game);
		break;
	case GME_State_EDITOR:
		GME_EDT_loop(game);
		break;
	default:
		break;
	}
}

void _main(void)
{
	// Reset non-const global variables in case the program is or was in RAM.
	ErrorInfo = NULL;

	// Memory leak detection
	u32 initial_mem = HeapAvail();

	// Universal game struct
	struct GME_Game game;
	COM_zero(&game);

	u16 error = 0;

	TRY
		GME_init(&game);

		// Run the mainloop. Let's get this show on the road!
		while (game.State != GME_State_NONE)
			GME_loop(&game);
	ONERR
		error = errCode;
	ENDTRY

	GME_deInit(&game);

	if (error)
		SCR_dispError(ErrorMessages[error], ErrorInfo);

	if (HeapAvail() != initial_mem)
		SCR_dispError("A memory leak has occurred. Please report",
				"this. See \"readme.txt\" for details.");
}
