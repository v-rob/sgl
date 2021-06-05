// Super Grayland: Copyright 2021 Vincent Robinson under the MIT license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#pragma once

#include "common.h"

#include "map.h"

// SCR Namespace: Screen
/*
	Everything in the SCR namespace operates on the screen. All functions that draw to the
	screen buffer itself are in this namespace. Ideally, no code should touch tigcclib drawing
	functions except for `screen.c`.

	All sprites, whether tiles or objects, are multiples of eight pixels in both axes. For
	tiles, this allows twenty tiles in the X direction and twelve in the Y with four pixels
	left for the HUD. Eight pixels is easy for the calculator to work with and allows plenty
	of space for gameplay on the 89's limited screen. And, as can be seen by Super Grayland's
	art, it's still possible to make things look very good. Objects can be greater than eight
	pixels by placing multiple sprites next to each other.

	All functions require the screen to be initialized, even if they don't take `SCR_Screen`
	as a parameter, unless otherwise stated.  Also, unless otherwise noted, they draw to
	the hidden buffer, not the active buffer.
*/

// The sprite size. This is preferred to a raw eight for semantic's sake.
#define SCR_SPRITE_SIZE 8

// The size of the 89's screen. TODO: No provisions have been made for the 92 or V200 as of yet.
#define SCR_SIZE_X 160
#define SCR_SIZE_Y 100

// Since the 89's screen height is not a multiple of eight, there are four extra pixels left
// over. This space is occupied by the HUD and the specially designed four pixel font.
#define SCR_HUD_HEIGHT 4

// This is the height of the part of the screen that holds the game, i.e. everything except
// the HUD.
#define SCR_GAME_HEIGHT (SCR_SIZE_Y - SCR_HUD_HEIGHT)

// Amount of sprites that can fit in the screen if all sprites are on a multiple of eight
// boundary.
#define SCR_SPRITES_X 20
#define SCR_SPRITES_Y 12

// Amount of sprites that can fit on the screen if sprites are scrolled to a non-multiple of
// eight boundary.
#define SCR_SCROLL_SPRITES_X (SCR_SPRITES_X + 1)
#define SCR_SCROLL_SPRITES_Y (SCR_SPRITES_Y + 1)

// Width of the screen buffer in bytes
#define SCR_SCREEN_BUFFER_WIDTH 30
// Height of the screen buffer in pixels
#define SCR_SCREEN_BUFFER_HEIGHT 128
// Size of the screen buffer in bytes
#define SCR_SCREEN_BUFFER_SIZE (SCR_SCREEN_BUFFER_WIDTH * SCR_SCREEN_BUFFER_HEIGHT)

// In a sprite buffer, each sprite is stored in the order dark -> light -> mask
#define SCR_SPRITE_DARK 0
#define SCR_SPRITE_LIGHT 1
#define SCR_SPRITE_MASK 2

// A buffer large enough to hold a single sprite.
typedef u8 SCR_SpriteBuffer[3][SCR_SPRITE_SIZE];

// A struct containing all data relevant to the screen.
struct SCR_Screen
{
	// Double buffering for grayscale
	u8 *GrayBuffer;

	// Tile buffer; see the SCR_TB namespace documentation for more info
	u8 *TileBuffer;

	// Pointers to the start of banks of sprites
	// Static tile sprite bank
	FILES TileBankFile;
	SCR_SpriteBuffer *TileBank;

	// Object sprite bank
	FILES ObjBankFile;
	SCR_SpriteBuffer *ObjBank;
};

// Defines the dimensions of something in pixels
typedef s16 SCR_Pixel;
#define SCR_Pixel_POINT 3

// Initializes the screen by starting grayscale, allocating screen buffers, and loading sprites.
void SCR_init(struct SCR_Screen *screen);
// Deinitializes the screen.
void SCR_deInit(struct SCR_Screen *screen);

// A "color" that can be provided to some drawing routines.
enum SCR_Color
{
	SCR_Color_WHITE,
	SCR_Color_LIGHT,
	SCR_Color_DARK,
	SCR_Color_BLACK
};

// Clears the screen. Generally unnecessary if using SCR_drawTileBuffer.
void SCR_clear(void);

// Clears both buffers.
void SCR_clearAll(void);

// Swaps hidden and active grayscale buffers.
#define SCR_swap() GrayDBufToggleSync()

// Draw a filled rectangle with the specified color.
void SCR_drawRect(SCR_Pixel x, SCR_Pixel y, SCR_Pixel w, SCR_Pixel h, enum SCR_Color color);

// Draw a tile directly to the screen without using the tile buffer.
void SCR_drawTile(struct SCR_Screen *screen, SCR_Pixel x, SCR_Pixel y,
		const struct MAP_TileDef *tile);

// Draws the tile buffer to the screen, shifting it a specified number of pixels to the top left
// corner of the screen, where the shift must be in the range [0, 7]. Drawing the tile buffer
// replaces the screen contents except for the HUD area, so clearing the screen is unnecessary.
void SCR_drawTileBuffer(struct SCR_Screen *screen, SCR_Pixel shift_left, SCR_Pixel shift_up);

// Scroll the map a certain amount, also shifting and updating the tile buffer appropriately
// as well.
void SCR_scroll(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Scroll shift_x,
		MAP_Scroll shift_y);

// Scroll the map to an absolute position, also updating the tile buffer appropriately as well.
void SCR_scrollAbsolute(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Scroll scroll_x,
		MAP_Scroll scroll_y);

#ifdef DEBUG
// Clears the screen, displays a debugging message with printf formatting, and waits for a
// keypress before returning. Draws to the active buffer.
void SCR_dispDebug(const char *format, ...);
#endif

// Clears the screen, displays an error, and waits for a keypress before returning. Should be
// used with grayscale off, i.e. when the screen is not initialized. Both `text1` and `text2`
// may be NULL.
void SCR_dispError(const char *text1, const char *text2);

// SCR_TB Namespace: Tile buffer sub-namespace for the screen
/*
	Super Grayland's tile system is heavy. Since there are two layers of tiles possible with
	four level grayscale and masking, this means that, if one were to clear and redraw the
	screen every frame, it would a) clear both buffers, b) draw two layers of tiles on
	potentially non-multiple of eight boundaries, c) draw two more layers, but this time
	masking the new tiles, making it four layers all told, and d) draw the objects and HUD
	after that. This is too much for the calculator, dropping to abominable framerates. Even
	when not drawing blank tiles, it's still too slow.

	So, the solution Super Grayland uses is to pre-draw all tiles on the screen to a separate
	buffer, the tile buffer. This essentially negates the cost of a, b, and c and replaces
	them with the single cost of copying the buffer to the gray buffer, which is incredibly
	cheaper. The only times the tile buffer has to be modified is when the screen scrolls over
	a tile boundary or when tiles animate. Still, the cost is negligible compared to redrawing
	multiple layers every frame.

	The tile buffer is a single chunk of memory holding a back and front buffer. Each buffer
	has enough space to hold SCR_SCROLL_SPRITES in the X and Y directions. Each sprite is
	aligned to a byte boundary and is only shifted pixel amounts when drawing the tile buffer
	to the screen. There is a catch: after each row of pixels, there is one blank padding byte
	to align each row to a u16 boundary, which makes `SCR_drawTileBuffer` much faster than
	single byte copying.
*/

// Note: WIDTH and SIZE are in bytes, while height is in other units, usually to be used as a
// multiplier for width.

// Width in bytes of one row of sprites
#define SCR_TB_SPRITES_WIDTH ((SCR_SCROLL_SPRITES_X + 1) * SCR_SPRITE_SIZE)
// Height in sprites of one plane
#define SCR_TB_SPRITES_HEIGHT SCR_SCROLL_SPRITES_Y

// Width in bytes of one row of pixels, including invisible alignment byte
#define SCR_TB_FULL_PLANE_WIDTH (SCR_SCROLL_SPRITES_X + 1)
// Width in bytes of one row of visible pixels.
#define SCR_TB_PLANE_WIDTH SCR_SCROLL_SPRITES_X
// Height in pixels of one plane
#define SCR_TB_PLANE_HEIGHT (SCR_SCROLL_SPRITES_Y * SCR_SPRITE_SIZE)

// Size in bytes of one plane
#define SCR_TB_PLANE_SIZE (SCR_TB_FULL_PLANE_WIDTH * SCR_TB_PLANE_HEIGHT)
// Size in bytes of the whole buffer
#define SCR_TB_BUFFER_SIZE (SCR_TB_PLANE_SIZE * 2)

// Defines the direction to shift in `SCR_TB_shift`.
enum SCR_TB_Dir
{
	SCR_TB_Dir_LEFT,
	SCR_TB_Dir_RIGHT,
	SCR_TB_Dir_UP,
	SCR_TB_Dir_DOWN,
};

// Shifts the tile buffer `amount` sprites in the direction `dir`. `amount` may not be greater
// than or equal to SCR_TB_PLANE_WIDTH if shifting horizontally or greater than
// SCR_TB_PLANE_HEIGHT if shifting vertically. It does not erase tiles in the space shifted out.
void SCR_TB_shift(struct SCR_Screen *screen, enum SCR_TB_Dir dir, u16 amount);

// Draws a tile to the tile buffer at the specified byte offset in the tile buffer. Overwrites
// any tile previously at that position.
void SCR_TB_drawTile(struct SCR_Screen *screen, const struct MAP_TileDef *tile, u16 offset);

// Draws a column of tiles from the map as tall as the tile buffer at the byte offset `x`.
// The top tile to draw is at position (`tile_x`, `tile_y`). Overwrites tiles previously
// at the positions being drawn to.
void SCR_TB_drawTileColumn(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y, u16 x);

// Draws a row of tiles from the map as wide as the tile buffer at the byte offset `y`.
// The top tile to draw is at position (`tile_x`, `tile_y`). Overwrites tiles previously
// at the positions being drawn to.
void SCR_TB_drawTileRow(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y, u16 y);

// Fills the tile buffer with tiles in the map where the top left tile is at position
// (`tile_x`, `tile_y`). Overwrites all tiles previously in the buffer.
void SCR_TB_drawAllTiles(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y);

// Redraws all animated tiles in the tile buffer.
void SCR_TB_updateAnimatedTiles(struct SCR_Screen *screen, struct MAP_Map *map,
		MAP_Pos tile_x, MAP_Pos tile_y);
