// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#include "game.h"
#include "map.h"
#include "player.h"
#include "screen.h"

struct game g_game;

bool init_game(void)
{
	// The game is currently starting up
	g_game.state = STATE_STARTUP;

	// Initialize player
	g_map.player = malloc(sizeof(struct player));
	if (g_map.player == NULL) {
		ERROR(ALLOC, "player");
		return FAILURE;
	}

	*g_map.player = (struct player) {
		.obj = {
			.type = OBJ_GRAYFORD,
			.pos = {Frac_NEW(pos_t, 6), Frac_NEW(pos_t, 8)},
		}
	};

	// Not strictly necessary, but shows what will be allocated
	g_map.tiles = malloc(0);
	g_map.objects = malloc(0);

	return SUCCESS;
}

void deinit_game(void)
{
	deinit_map();
}

bool mainloop(void)
{
	// Frame start time for frame rate capping
	u32 frame_start = SDL_GetTicks();

	// Handle Alt-F4 or the close button
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			g_game.state = STATE_QUIT;
			return SUCCESS;
		}
	}

	// Get keys for passing to helper functions
	const u8 *keys = SDL_GetKeyboardState(NULL);

	// Depending on the current state, do different things
	switch (g_game.state) {
	// The mainloop should never be in the quit state
	case STATE_QUIT:
		S_ERROR(UNKNOWN);
		return FAILURE;
		break;

	case STATE_STARTUP:
		break;

	case STATE_LEVEL:
		update_player(keys);
		draw_screen();
		break;
	}

	// Cap the frame rate
	u32 diff = SDL_GetTicks() - frame_start;
	if (diff < TIME_PER_FRAME)
		SDL_Delay(TIME_PER_FRAME - diff);

	return SUCCESS;
}
