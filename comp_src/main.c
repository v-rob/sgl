// Super Grayland, Copyright 2020 Vincent Robinson under the zlib license. See 'LICENSE.txt' for more information.

// Please see 'common.h' for documentation on the types and important nuances of Super Grayland's source code.
// All other headers have detailed information on globals, functions, and constants. Implementations are highly commented.

#include "common.h"
#include "game.h"
#include "map.h"
#include "screen.h"

int main(void)
{
	printf("--------------------------------------------------\n" // TODO: Careful with s32 and %d; they might not be the same
			"Super Grayland v1.0\n"
			"--------------------------------------------------\n"
			"Copyright 2020 Vincent Robinson\n"
			"See LICENSE.txt for licensing information\n\n"
			"Website: v-rob.github.io/sgl\n\n"
			"Log:\n"
			"--------------------------------------------------\n"
	);

	// Initialize
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		S_ERROR(SDL_INIT);
		goto deinit_sdl;
	}

	if (init_display())
		goto deinit_display;
	if (init_game()) // TODO: Freeing?
		goto deinit_game;

	if (init_map())
		goto deinit_game;

	// Run main loop
	while (g_game.state != STATE_QUIT) {
		if (mainloop()) {
			g_game.state = STATE_QUIT;
			break;
		}
	}

	// Deinitialize
deinit_game:
	deinit_game();
deinit_display: // TODO: Correct cleanup
	deinit_display();

deinit_sdl:
	SDL_Quit();

	if (g_error_no)
		log_error("Super Grayland exited with with an error.\n"
				"--------------------------------------------------\n");
	else
		printf("Super Grayland exited successfully.\n"
				"--------------------------------------------------\n");

	return g_error_no;
}
