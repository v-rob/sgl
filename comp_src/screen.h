// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#pragma once

#include "common.h"

/* Fixed/integer types:
	* scroll_t is a 29.3 fixed point number defining a scrolled position on the screen. It can represent any pixel position
	  on the screen.
	* pixel_t is a width/height of something in pixels. Although it is a 5.3 fixed point type, it is not necessary to use
	  Frac_NEW_FRAC with it because it is simply in pixels, so integers work fine.
	* scroll_t and pixel_t are compatible with each other, so no fixed point conversion is necessary between them.
	* sprite_t is like tile_t, but it is only 8 bits and should be used with spritesheets and tile positions on the screen,
	  not absolute tile positions.
	* tile_t and sprite_t are compatible with each other, so no fixed point conversion is necessary between them.
*/

TYPEDEF(s32, scroll_t)
#define scroll_t_POINT 3
TYPEDEF(u8, pixel_t)
#define pixel_t_POINT 3
TYPEDEF(s8, sprite_t)
#define sprite_t_POINT 0

// Some size constants
#define TILE_SIZE 8
#define HUD_HEIGHT 4

// Size of the screen in tiles
#define SCREEN_TILES_X 20
#define SCREEN_TILES_Y 12
// How many tiles can fit on the screen when scrolled to a non-tile multiple
#define SCROLL_SCREEN_TILES_X (SCREEN_TILES_X + 1)
#define SCROLL_SCREEN_TILES_Y (SCREEN_TILES_Y + 1)

// Screen dimensions in physical pixels
#define SCREEN_WIDTH  (SCREEN_TILES_X * TILE_SIZE)
#define SCREEN_HEIGHT (SCREEN_TILES_Y * TILE_SIZE + HUD_HEIGHT)

// Screen scaling convenience macro
#define SCALE(N) ((N) * m_display.scale)

// FPS cap stuff
#define MAX_FPS 30
#define TIME_PER_FRAME (1000 / MAX_FPS)

// Default frames between updates for normal animation speeds
#define ANI_UPDATE_FRAMES 2

// Colors, if they can be called that TODO: Optimize so the function isn't called every time
#define COLOR_WHITE SDL_MapRGB(m_display.screen->format, 0xC4, 0xD6, 0xC4)
#define COLOR_LIGHT SDL_MapRGB(m_display.screen->format, 0x94, 0x9E, 0x8C)
#define COLOR_DARK  SDL_MapRGB(m_display.screen->format, 0x64, 0x66, 0x5C)
#define COLOR_BLACK SDL_MapRGB(m_display.screen->format, 0x34, 0x2E, 0x24)
#define COLOR_NONE  SDL_MapRGB(m_display.screen->format, 0xFF, 0xFF, 0xFF)

// Initialize/deinitialize the entire display and surfaces
bool init_display(void);
void deinit_display(void);

struct obj;

// Draw an object to the screen surface
void draw_obj(const struct obj *obj);

// Draw all tiles on a specified layer to the screen surface
void draw_tiles(bool fg);

// Scroll the screen to center on the player
void scroll_to_player(void);

// Wipe everything on the screen surface to white
void clear_screen(void);

// Show everything on the screen surface to the physical screen
void update_window(void);

// Draw everything to the screen
void draw_screen(void);
