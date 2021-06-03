// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#include "map.h"
#include "game.h"
#include "map.h"
#include "obj.h"
#include "player.h"
#include "screen.h"

// Struct containing all screen-related stuff
static struct
{
	// Actual window
	SDL_Window *window;
	// Screen surface which everything is blitted to which is attached to the window
	SDL_Surface *screen;

	// Surface for flipping a sprite
	SDL_Surface *flipper;

	// Loaded spritesheet surfaces
	SDL_Surface *obj_sheet;
	SDL_Surface *tile_sheet;
	SDL_Surface *letter_sheet;

	// Current animation frames for 2, 3, and 4 frame tiles.
	u8 ani_frames[3];
	// Wait time counter between frames of 'ANI_UPDATE_FRAMES' frames.
	u8 ani_update;

	// How many physical screen pixels one sprite pixel is
	u8 scale;
} m_display;

// Sizes of the tile sheets
#define OBJ_SHEET_WIDTH 8
#define TILE_SHEET_WIDTH 16

// Prepare a surface for use with blitting
static SDL_Surface *init_surface(SDL_Surface *surface, char name[])
{
	// Convert sprites to optimized and scaleable surface
	surface = SDL_ConvertSurface(surface, m_display.screen->format, 0);
	if (surface == NULL) {
		ERROR(SDL_CONVERT_SURFACE, name);
		return NULL;
	}

	// Set sprite transparency
	if (SDL_SetColorKey(surface, true, COLOR_NONE)) {
		ERROR(SDL_SET_COLOR_KEY, name);
		return NULL;
	}

	return surface;
}

// Load a spritesheet from the filesystem
static SDL_Surface *load_spritesheet(char path[], char name[])
{
	SDL_Surface *surface = NULL;

	// Load spritesheet
	surface = SDL_LoadBMP(path);
	if (surface == NULL) {
		ERROR(SDL_LOAD_MEDIA, path);
		return NULL;
	}

	// Prepare surface for scaled blitting, etc.
	surface = init_surface(surface, name);

	return surface;
}

bool init_display(void)
{
	m_display.scale = 4;

	// Create the window
	m_display.window = SDL_CreateWindow("Super Grayland", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCALE(SCREEN_WIDTH), SCALE(SCREEN_HEIGHT), SDL_WINDOW_SHOWN);
	if (m_display.window == NULL) {
		S_ERROR(SDL_CREATE_WINDOW);
		SDL_Quit();
		return FAILURE;
	}

	// Get the screen surface
	m_display.screen = SDL_GetWindowSurface(m_display.window);

	// Load spritesheets
	m_display.obj_sheet = load_spritesheet("media/obj.bmp", "object spritesheet");
	m_display.tile_sheet = load_spritesheet("media/tile.bmp", "tile spritesheet");
	m_display.letter_sheet = load_spritesheet("media/letter.bmp", "letter spritesheet");

	if (m_display.obj_sheet == NULL || m_display.tile_sheet == NULL || m_display.letter_sheet == NULL)
		return FAILURE;

	// Set up sprite flipping surface
	m_display.flipper = SDL_CreateRGBSurface(0, 24, 16, m_display.screen->format->BitsPerPixel, 0, 0, 0, 0);
	if (m_display.flipper == NULL) {
		ERROR(SDL_CREATE_SURFACE, "sprite flipper");
		return FAILURE;
	}

	m_display.flipper = init_surface(m_display.flipper, "sprite flipper");
	if (m_display.flipper == NULL)
		return FAILURE;

	return SUCCESS;
}

void deinit_display(void)
{
	SDL_FreeSurface(m_display.screen);
	SDL_FreeSurface(m_display.obj_sheet);
	SDL_FreeSurface(m_display.tile_sheet);
	SDL_FreeSurface(m_display.letter_sheet);

	SDL_DestroyWindow(m_display.window);
}

void draw_obj(const struct obj *obj)
{
	const struct obj_def *def = get_obj_def(obj->type);

	// Where the tile is on the spritesheet
	SDL_Rect sheet_rect = {
		def->sprite_pos.x * TILE_SIZE,
		def->sprite_pos.y * TILE_SIZE,
		TILE_SIZE * (def->extra_sprite.x + 1),
		TILE_SIZE * (def->extra_sprite.y + 1)
	};

	// Factor in animations
	if (def->ani_vert)
		sheet_rect.y += obj->sprite_offset * (def->extra_sprite.y + 1) * TILE_SIZE;
	else
		sheet_rect.x += obj->sprite_offset * (def->extra_sprite.x + 1) * TILE_SIZE;

	// Where to blit the object onto the screen
	SDL_Rect screen_rect = {
		SCALE(Frac_CONVERT(pos_t, scroll_t, obj->pos.x) - g_map.scroll.x),
		SCALE(Frac_CONVERT(pos_t, scroll_t, obj->pos.y) - g_map.scroll.y + HUD_HEIGHT),
		SCALE(sheet_rect.w),
		SCALE(sheet_rect.h)
	};

	// Which surface to blit from
	SDL_Surface *from_blit = m_display.obj_sheet;

	// Flip the sprite if necessary
	if (obj->flip_x || obj->flip_y) {
		// Clear flipper surface
		SDL_FillRect(m_display.flipper, NULL, COLOR_NONE);

		u32 *sheet_pixels = m_display.obj_sheet->pixels;
		u32 *flipper_pixels = m_display.flipper->pixels;

		// Set flipped pixels
		if (obj->flip_x) {
			for (u8 x = 0, rx = sheet_rect.w - 1; x < sheet_rect.w; x++, rx--)
			for (u8 y = 0; y < sheet_rect.h; y++)
				flipper_pixels[y * m_display.flipper->w + rx] = sheet_pixels[(y + sheet_rect.y) * m_display.obj_sheet->w + x + sheet_rect.x];
		}
		if (obj->flip_y) {
			for (u8 x = 0; x < sheet_rect.w; x++)
			for (u8 y = 0, ry = sheet_rect.h - 1; y < sheet_rect.h; y++, ry--)
				flipper_pixels[ry * m_display.flipper->w + x] = sheet_pixels[(y + sheet_rect.y) * m_display.obj_sheet->w + x + sheet_rect.x];
		}

		// Set to blit from the flipper
		from_blit = m_display.flipper;
		sheet_rect.x = sheet_rect.y = 0; // Set to 0 for use with flipper surface since there is no offset
	}

	// Handle wrap-around drawing
	if (g_map.wrap_horiz == WRAP_OBJS || g_map.wrap_vert == WRAP_OBJS) {
		// Only draw again if the object is currently wrapped at the edge
		bool draw_horiz = g_map.wrap_horiz && obj->pos.x > Frac_CONVERT(tile_t, pos_t, g_map.size.x - def->extra_sprite.x - 1);
		bool draw_vert  = g_map.wrap_vert  && obj->pos.y > Frac_CONVERT(tile_t, pos_t, g_map.size.y - def->extra_sprite.y - 1);

		v2_s16 other = {0, 0};

		if (draw_horiz) {
			SDL_Rect other_rect = screen_rect;
			other_rect.x = other.x = SCALE(Frac_CONVERT(pos_t, scroll_t, obj->pos.x) -
					Frac_CONVERT(tile_t, scroll_t, g_map.size.x) - g_map.scroll.x);
			SDL_BlitScaled(from_blit, &sheet_rect, m_display.screen, &other_rect);
		}
		if (draw_vert) {
			SDL_Rect other_rect = screen_rect;
			other_rect.y = other.y = SCALE(Frac_CONVERT(pos_t, scroll_t, obj->pos.y) -
					Frac_CONVERT(tile_t, scroll_t, g_map.size.y) - g_map.scroll.y + HUD_HEIGHT);
			SDL_BlitScaled(from_blit, &sheet_rect, m_display.screen, &other_rect);
		}
		if (draw_horiz && draw_vert) {
			SDL_Rect other_rect = screen_rect;
			other_rect.x = other.x;
			other_rect.y = other.y;
			SDL_BlitScaled(from_blit, &sheet_rect, m_display.screen, &other_rect);
		}
	}

	// Always blit the main object at its normal (not wrapped if wrapping is enabled) position
	SDL_BlitScaled(from_blit, &sheet_rect, m_display.screen, &screen_rect);
}

void draw_tiles(bool fg)
{
	v2_tile_t top_left_tile = {
		Frac_CONVERT(scroll_t, tile_t, g_map.scroll.x),
		Frac_CONVERT(scroll_t, tile_t, g_map.scroll.y)
	};

	for (sprite_t x = 0; x < SCROLL_SCREEN_TILES_X; x++)
	for (sprite_t y = 0; y < SCROLL_SCREEN_TILES_Y; y++) {
		const struct tile *tile = get_tile(x + top_left_tile.x, y + top_left_tile.y);

		// Draw as little as possible to the screen; this will perform faster
		if (tile->front == TILE_AIR && tile->back == TILE_AIR)
			continue;

		SDL_Rect sheet_rect = {
			tile->back % TILE_SHEET_WIDTH * TILE_SIZE,
			tile->back / TILE_SHEET_WIDTH * TILE_SIZE,
			TILE_SIZE,
			TILE_SIZE
		};

		SDL_Rect screen_rect = {
			SCALE(x * TILE_SIZE - Frac_FRAC_PART(scroll_t, g_map.scroll.x)),
			SCALE(y * TILE_SIZE + 4 - Frac_FRAC_PART(scroll_t, g_map.scroll.y)),
			SCALE(TILE_SIZE),
			SCALE(TILE_SIZE)
		};

		if (tile->back != TILE_AIR && fg == tile->is_back_fg) {
			// A copy prevents SDL rect repairing from messing up front layer drawing
			SDL_Rect screen_rect_copy = screen_rect;
			SDL_BlitScaled(m_display.tile_sheet, &sheet_rect, m_display.screen, &screen_rect_copy);
		}

		if (tile->front != TILE_AIR && fg == tile->is_front_fg) {
			sheet_rect.x = tile->front % TILE_SHEET_WIDTH * TILE_SIZE,
			sheet_rect.y = tile->front / TILE_SHEET_WIDTH * TILE_SIZE,
			SDL_BlitScaled(m_display.tile_sheet, &sheet_rect, m_display.screen, &screen_rect);
		}
	}
}

void scroll_to_player(void)
{
	const struct obj_def *def = get_obj_def(g_map.player->obj.type);

	// Get the player pos to center on
	v2_scroll_t pos = {
		Frac_CONVERT(pos_t, scroll_t, g_map.player->obj.pos.x),
		Frac_CONVERT(pos_t, scroll_t, g_map.player->obj.pos.y)
	};

	// Yuk, this is some darn ugly code! There's nothing for it, though

	// Make sure the scrolling never leaves the map except if the level is object wrap-around
	if (g_map.wrap_horiz != WRAP_LEVEL && pos.x <= Frac_NEW_FRAC(scroll_t, SCREEN_TILES_X / 2 - 1, 1, 2)) {
		g_map.scroll.x = 0;
	} else if (g_map.wrap_horiz != WRAP_LEVEL &&
			pos.x + Frac_NEW_FRAC(scroll_t, SCREEN_TILES_X / 2, 1, 2) >= Frac_CONVERT(tile_t, scroll_t, g_map.size.x))
	{
		// Wrap from front if halfway across level boundaries in object wrap-around
		if (g_map.wrap_horiz == WRAP_OBJS && pos.x >= Frac_CONVERT(tile_t, scroll_t, g_map.size.x) - get_rect_center(def).x)
			g_map.scroll.x = 0;
		else
			g_map.scroll.x = Frac_CONVERT(tile_t, scroll_t, g_map.size.x - SCREEN_TILES_X);
	} else {
		// Otherwise keep player centered
		g_map.scroll.x = pos.x - Frac_NEW_FRAC(scroll_t, SCREEN_TILES_X / 2 - 1, 1, 2);
	}

	// Vertical scrolling is almost identical to horizontal, but we have to make sure the tall and small character stay
	// at the same level. Scrolling when the character got or lost a powerup would be bad.
	if (g_map.wrap_vert != WRAP_LEVEL && pos.y <= Frac_NEW_FRAC(scroll_t, SCREEN_TILES_Y / 2 - def->extra_sprite.y - 1, 1, 2)) {
		g_map.scroll.y = 0;
	} else if (g_map.wrap_vert != WRAP_LEVEL &&
			pos.y + Frac_NEW_FRAC(scroll_t, SCREEN_TILES_Y / 2 + def->extra_sprite.y, 1, 2) >= Frac_CONVERT(tile_t, scroll_t, g_map.size.y))
	{
		if (g_map.wrap_vert == WRAP_OBJS && pos.y >= Frac_CONVERT(tile_t, scroll_t, g_map.size.y) - get_rect_center(def).y)
			g_map.scroll.y = 0;
		else
			g_map.scroll.y = Frac_CONVERT(tile_t, scroll_t, g_map.size.y - SCREEN_TILES_Y);
	} else {
		g_map.scroll.y = pos.y - Frac_NEW_FRAC(scroll_t, SCREEN_TILES_Y / 2 - def->extra_sprite.y - 1, 1, 2);
	}
}

void clear_screen(void)
{
	SDL_FillRect(m_display.screen, NULL, COLOR_WHITE);
}

void update_window(void)
{
	SDL_UpdateWindowSurface(m_display.window);
}

void draw_screen(void)
{
	clear_screen();

	// Update all tile animation frames on the animation update frame
	WRAP_ADD(m_display.ani_update, +1, ANI_UPDATE_FRAMES, 0);
	if (m_display.ani_update == 0) {
		for (u8 i = 0; i < 3; i++)
			WRAP_ADD(m_display.ani_frames[i], +1, i + 1, 0);
	}

	scroll_to_player();

	// Background tiles
	draw_tiles(false);

	// Draw all objects TODO: Only draw on screen
	for (struct obj *obj = get_obj(g_map.left_active_index); obj != NULL; obj = get_right_obj(obj, g_map.right_active_index))
		draw_obj(obj);

	// Draw the player after the other objects; we always want the player in front
	draw_obj(&g_map.player->obj);

	// Foreground tiles
	draw_tiles(true);

	// Draw space for HUD
	SDL_Rect rect = {0, 0, SCALE(SCREEN_WIDTH), SCALE(4)};
	SDL_FillRect(m_display.screen, &rect, COLOR_WHITE);

	update_window();
}
