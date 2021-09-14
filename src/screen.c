// Super Grayland: Copyright 2021 Vincent Robinson under the zlib license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#include "object.h"
#include "screen.h"

// Temporary sprites; sprites will be externalized to a file later.
const SCR_SpriteBuffer TEMP_TILE_SPRITES[] = {
	{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
	},
	{
		{0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF},
		{0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF},
		{0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00}
	},
	{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x24, 0x24, 0xE7, 0x00, 0x00, 0xE7, 0x24, 0x24},
		{0xDB, 0xDB, 0x18, 0xFF, 0xFF, 0x18, 0xDB, 0xDB}
	},
	{
		{0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x7E, 0xBD, 0xDB, 0xE7, 0xE7, 0xDB, 0xBD, 0x7E}
	},
};

// "SUPER GRAYLAND" text for large screens. Disgusting for now, but will be put into an external
// file later, along with all the other sprites.
const u16 TEMP_TITLE[97 * 2] = {
	0b1110000000000111, 0b1100000000000011, 0b1100111111110011, 0b1100111111110011, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1110000000000011, 0b1100000000000011, 0b1100111111111111, 0b1100111111111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100000000000011, 0b1100000000000011, 0b1100111001111111, 0b1100111001111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1111111111110011, 0b1111111111110011, 0b1111111111110011, 0b1111111111110011, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100000001111111, 0b1100000001111111, 0b1111111000000011, 0b1111111000000011, 0b1100000001111111, 0b1100000001111111, 0b1111111111111111, 0b1100000000000011, 0b1100000000000011, 0b1100111001111111, 0b1100111001111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100000001110011, 0b1100000001000011, 0b1100111000001111, 0b1100111000111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100111000000011, 0b1100111000000011, 0b1100111001110011, 0b1100111111110011, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1111111111111111, 0b1100000001110011, 0b1100000001000011, 0b1100111000001111, 0b1100111000111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100111111110011, 0b1100111001110011, 0b1100111001110011, 0b1100111001110011, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100000001111111, 0b1100000001111111, 0b1100111001111111, 0b1100111001111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100000000000011, 0b1100000000000011, 0b1111111111110011, 0b1111111111110011, 0b1100000000000011, 0b1100000000000011, 0b1111111111111111, 0b1100111000000011, 0b1100111000000011, 0b1100111001110011, 0b1100111001110011, 0b1100000001110011, 0b1100000001110011
};

// TODO: Garbage collect or alloc high?
void SCR_init(struct SCR_Screen *screen)
{
	// Most things in SGL that use a system font use the small font.
	FontSetSys(F_4x6);

	if (!GrayOn())
		COM_throwErr(COM_Error_OTHER, "Grayscale could not be initialized.");

	screen->GrayBuffer = HeapAllocPtr(GRAYDBUFFER_SIZE);
	if (screen->GrayBuffer == NULL)
		COM_throwErr(COM_Error_MEMORY, "grayscale buffer");
	GrayDBufInit(screen->GrayBuffer);

	SCR_drawBorder();

	screen->TileBuffer = HeapAllocPtr(SCR_TB_BUFFER_SIZE);
	if (screen->TileBuffer == NULL)
		COM_throwErr(COM_Error_MEMORY, "tile buffer");

	/* if (FOpen("sgl\\tiles", &screen->TileBankFile, FM_READ, "sgls") != FS_OK)
		COM_throwErr(COM_Error_FILE, "sgl\\tiles");
	screen->TileBank = HeapDeref(screen->TileBankFile.dataH);
	if (FOpen("sgl\\objects", &screen->ObjBankFile, FM_READ, "sgls") != FS_OK)
		COM_throwErr(COM_Error_FILE, "sgl\\objects");
	screen->ObjBank = HeapDeref(screen->ObjBankFile.dataH) */

	screen->TileBank = TEMP_TILE_SPRITES;

	return;
}

void SCR_deInit(struct SCR_Screen *screen)
{
	GrayOff(); // This can be called even when grayscale is already off.

	if (screen->GrayBuffer != NULL)
		HeapFreePtr(screen->GrayBuffer);
	if (screen->TileBuffer != NULL)
		HeapFreePtr(screen->TileBuffer);

	// FClose(&screen->TileBankFile);
	// FClose(&screen->ObjBankFile);

	COM_zero(screen);
}

void SCR_clear(void)
{
	if (SCR_isLargeScreen()) {
		u8 *dark  = GrayDBufGetHiddenPlane(DARK_PLANE);
		u8 *light = GrayDBufGetHiddenPlane(LIGHT_PLANE);

		for (u16 i = SCR_LARGE_OFFSET_BYTES; i < SCR_LARGE_OFFSET_BYTES_END;
				i += SCR_SCREEN_BUFFER_WIDTH) {
			memset(dark  + i, 0, SCR_WIDTH_BYTES);
			memset(light + i, 0, SCR_WIDTH_BYTES);
		}
	} else {
		GrayDBufSetHiddenAMSPlane(LIGHT_PLANE);
		ClrScr();
		GrayDBufSetHiddenAMSPlane(DARK_PLANE);
		ClrScr();
	}
}

static void redrawBorderAndTitle(void)
{
	u8 *dark  = GrayDBufGetHiddenPlane(DARK_PLANE);
	u8 *light = GrayDBufGetHiddenPlane(LIGHT_PLANE);

	memset(dark,  0xFF, SCR_LARGE_OFFSET_BYTES);
	memset(light, 0xFF, SCR_LARGE_OFFSET_BYTES);

	for (u16 i = SCR_LARGE_OFFSET_BYTES; i < SCR_LARGE_OFFSET_BYTES_END;
				i += SCR_SCREEN_BUFFER_WIDTH) {
		memset(dark  + i + SCR_WIDTH_BYTES, 0xFF, SCR_SCREEN_BUFFER_WIDTH - SCR_WIDTH_BYTES);
		memset(light + i + SCR_WIDTH_BYTES, 0xFF, SCR_SCREEN_BUFFER_WIDTH - SCR_WIDTH_BYTES);
	}

	memset(dark  + SCR_LARGE_OFFSET_BYTES_END, 0xFF,
			SCR_SCREEN_BUFFER_SIZE - SCR_LARGE_OFFSET_BYTES_END);
	memset(light + SCR_LARGE_OFFSET_BYTES_END, 0xFF,
			SCR_SCREEN_BUFFER_SIZE - SCR_LARGE_OFFSET_BYTES_END);

	Sprite16(16, 16, 97, TEMP_TITLE, dark,  SPRT_RPLC);
	Sprite16(16, 16, 97, TEMP_TITLE, light, SPRT_RPLC);
}

void SCR_drawBorder(void)
{
	bool is_large = SCR_isLargeScreen();

	if (is_large)
		redrawBorderAndTitle();
	SCR_clear();

	SCR_swap();

	if (is_large)
		redrawBorderAndTitle();
	SCR_clear();
}

void SCR_drawRect(SCR_Pixel x, SCR_Pixel y, SCR_Pixel w, SCR_Pixel h, enum SCR_Shade color)
{
	SCR_RECT rect = {{
		x + SCR_LARGE_OFFSET_X,
		y + SCR_LARGE_OFFSET_Y,
		x + SCR_LARGE_OFFSET_X + w,
		y + SCR_LARGE_OFFSET_Y + h
	}};

	GrayDBufSetHiddenAMSPlane(DARK_PLANE);
	if (color == SCR_Shade_DARK || color == SCR_Shade_BLACK)
		ScrRectFill(&rect, &rect, A_NORMAL);
	else
		ScrRectFill(&rect, &rect, A_REVERSE);

	GrayDBufSetHiddenAMSPlane(LIGHT_PLANE);
	if (color == SCR_Shade_LIGHT || color == SCR_Shade_BLACK)
		ScrRectFill(&rect, &rect, A_NORMAL);
	else
		ScrRectFill(&rect, &rect, A_REVERSE);
}

void SCR_drawTile(const struct SCR_Screen *screen, SCR_Pixel x, SCR_Pixel y,
		const struct MAP_TileDef *tile)
{
	x += SCR_LARGE_OFFSET_X;
	y += SCR_LARGE_OFFSET_Y;

	u8 *dark  = GrayDBufGetHiddenPlane(DARK_PLANE);
	u8 *light = GrayDBufGetHiddenPlane(LIGHT_PLANE);

	const SCR_SpriteBuffer *bank = screen->TileBank;

	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Back][SCR_SPRITE_MASK], dark, SPRT_AND);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Back][SCR_SPRITE_DARK], dark, SPRT_OR);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Back][SCR_SPRITE_MASK], light, SPRT_AND);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Back][SCR_SPRITE_LIGHT], light, SPRT_OR);

	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Front][SCR_SPRITE_MASK], dark, SPRT_AND);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Front][SCR_SPRITE_DARK], dark, SPRT_OR);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Front][SCR_SPRITE_MASK], light, SPRT_AND);
	ClipSprite8(x, y, SCR_SPRITE_SIZE, bank[tile->Front][SCR_SPRITE_LIGHT], light, SPRT_OR);
}

void SCR_drawObject(const struct OBJ_Object *obj)
{
	const struct OBJ_ObjectDef *def = OBJ_getDef(obj->Type);
	(void)def;
	// TODO: This needs to be implemented
}

// TODO: Only shift in one direction? Could speed it up.
void SCR_drawTileBuffer(const struct SCR_Screen *screen, SCR_Pixel shift_left, SCR_Pixel shift_up)
{
	// There are a lot of divisions by two because this function uses u16s instead of u8s, which
	// are _vaaastly_ faster, tripling the framerate. Potentially, u32s might be faster, but
	// the screen buffer rows are 30 bytes, which is not divisible by four, so there's no way
	// to use them.

	u16 *dark  = GrayDBufGetHiddenPlane(DARK_PLANE);
	u16 *light = GrayDBufGetHiddenPlane(LIGHT_PLANE);

	s16 v_shift = shift_up * SCR_TB_FULL_PLANE_WIDTH;

	// Tile buffer counter and end position
	u16 *it = (u16 *)screen->TileBuffer + v_shift / 2;
	u16 *it_end = (u16 *)screen->TileBuffer +
			(SCR_TB_PLANE_SIZE - (SCR_SPRITE_SIZE - v_shift) - SCR_TB_SPRITES_WIDTH) / 2;

	// Screen buffer counter
	u16 is = SCR_SCREEN_BUFFER_WIDTH / 2 * SCR_HUD_HEIGHT;

	if (SCR_isLargeScreen())
		is += SCR_LARGE_OFFSET_BYTES / 2;

	for (; it < it_end; it += SCR_TB_FULL_PLANE_WIDTH / 2, is += SCR_SCREEN_BUFFER_WIDTH / 2) {
		for (u16 i = 0; i < (SCR_TB_PLANE_WIDTH - 1) / 2; i++) {
			// Leave as-is. GCC optimizes it better all together like this instead of in
			// separate variables.
			*(dark + is + i) = (*(it + i) << shift_left) |
					(*(it + i + 1) >> (16 - shift_left));
			*(light + is + i) = (*(it + i + SCR_TB_PLANE_SIZE / 2) << shift_left) |
					(*(it + i + SCR_TB_PLANE_SIZE / 2 + 1) >> (16 - shift_left));
		}
	}
}

void SCR_scroll(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Scroll shift_x,
		MAP_Scroll shift_y)
{
	MAP_Scroll old_scroll_x = map->ScrollX;
	map->ScrollX += shift_x;

	// If we scrolled over a tile boundary after the addition, the tile buffer needs to be
	// shifted and the empty column filled with tiles.
	if (FXD_floor(MAP_Scroll, map->ScrollX) - FXD_floor(MAP_Scroll, old_scroll_x) > 0) {
		SCR_TB_shift(screen, SCR_TB_Dir_LEFT, 1);
		SCR_TB_drawTileColumn(screen, map,
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollX) + SCR_SPRITES_X,
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollY),
				SCR_TB_PLANE_WIDTH - 1);
	} else if (FXD_floor(MAP_Scroll, map->ScrollX) - FXD_floor(MAP_Scroll, old_scroll_x) < 0) {
		SCR_TB_shift(screen, SCR_TB_Dir_RIGHT, 1);
		SCR_TB_drawTileColumn(screen, map,
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollX),
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollY),
				0);
	}

	// This must be added _after_ shifting the X or else it messes up the shifting code.
	MAP_Scroll old_scroll_y = map->ScrollY;
	map->ScrollY += shift_y;

	if (FXD_floor(MAP_Scroll, map->ScrollY) - FXD_floor(MAP_Scroll, old_scroll_y) > 0) {
		SCR_TB_shift(screen, SCR_TB_Dir_UP, 1);
		SCR_TB_drawTileRow(screen, map,
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollX),
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollY) + SCR_SPRITES_Y,
				SCR_TB_PLANE_SIZE - SCR_TB_SPRITES_WIDTH);
	} else if (FXD_floor(MAP_Scroll, map->ScrollY) - FXD_floor(MAP_Scroll, old_scroll_y) < 0) {
		SCR_TB_shift(screen, SCR_TB_Dir_DOWN, 1);
		SCR_TB_drawTileRow(screen, map,
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollX),
				FXD_convert(MAP_Scroll, MAP_Pos, map->ScrollY),
				0);
	}
}

void SCR_scrollAbsolute(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Scroll scroll_x,
		MAP_Scroll scroll_y)
{
	map->ScrollX = scroll_x;
	map->ScrollY = scroll_y;

	SCR_TB_drawAllTiles(screen, map,
			FXD_convert(MAP_Scroll, MAP_Pos, scroll_x), FXD_convert(MAP_Scroll, MAP_Pos, scroll_x));
}

#ifdef DEBUG
void SCR_dispDebug(const char *format, ...)
{
	ClrScr();

	va_list arglist;
	va_start(arglist, format);

	printf("\n");
	vprintf(format, arglist);

	va_end(arglist);

	DrawStr(1, 1, "Hey presto! A debugging message for you:", A_NORMAL);

	COM_waitForKey();

	SCR_drawBorder();
}
#endif

void SCR_dispError(const char *error, const char *info)
{
	ClrScr();
	DrawStr(1, 1, "A problem occurred. Sorry! :(", A_NORMAL);

	u16 place = 11;
	if (error != NULL) {
		DrawStr(1, place, error, A_NORMAL);
		place += 7;
	}

	if (info != NULL) {
		DrawStr(1, place, info, A_NORMAL);
		place += 7;
	}

	DrawStr(1, place + 4, "(It's probably Gravil's fault.)", A_NORMAL);
	DrawStr(1, place + 11, "Press the any key to exit.", A_NORMAL);

	COM_waitForKey();
}

void SCR_TB_shift(struct SCR_Screen *screen, enum SCR_TB_Dir dir, u16 amount)
{
	u8 *buf = screen->TileBuffer;
	switch (dir) {
	case SCR_TB_Dir_LEFT:
		for (u8 *i = buf; i < buf + SCR_TB_BUFFER_SIZE; i += SCR_TB_FULL_PLANE_WIDTH)
			memmove(i, i + amount, SCR_TB_PLANE_WIDTH - amount);
		break;
	case SCR_TB_Dir_RIGHT:
		for (u8 *i = buf; i < buf + SCR_TB_BUFFER_SIZE; i += SCR_TB_FULL_PLANE_WIDTH)
			memmove(i + amount, i, SCR_TB_PLANE_WIDTH - amount);
		break;
	case SCR_TB_Dir_UP:
		{
			u16 offset = SCR_TB_SPRITES_WIDTH * amount;
			memmove(buf, buf + offset, SCR_TB_BUFFER_SIZE - offset);
		}
		break;
	case SCR_TB_Dir_DOWN:
		{
			u16 offset = SCR_TB_SPRITES_WIDTH * amount;
			memmove(buf + offset, buf, SCR_TB_BUFFER_SIZE - offset);
		}
		break;
	}
}

void SCR_TB_drawTile(struct SCR_Screen *screen, const struct MAP_TileDef *tile, u16 offset)
{
	u8 *dark = screen->TileBuffer + offset;
	u8 *light = dark + SCR_TB_PLANE_SIZE;

	const SCR_SpriteBuffer *bank = screen->TileBank;

	// Air is intentionally drawn because tiles shifted out in SCR_TB_shift are not erased, so
	// air will erase them.
	for (u16 row = 0, offset = 0; row < SCR_SPRITE_SIZE;
			row++, offset += SCR_TB_FULL_PLANE_WIDTH) {
		*(dark + offset) = bank[tile->Back][SCR_SPRITE_DARK][row];
		*(light + offset) = bank[tile->Back][SCR_SPRITE_LIGHT][row];

		*(dark + offset) &= bank[tile->Front][SCR_SPRITE_MASK][row];
		*(dark + offset) |= bank[tile->Front][SCR_SPRITE_DARK][row];
		*(light + offset) &= bank[tile->Front][SCR_SPRITE_MASK][row];
		*(light + offset) |= bank[tile->Front][SCR_SPRITE_LIGHT][row];
	}
}

void SCR_TB_drawTileColumn(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y, u16 x)
{
	s16 end = tile_y + SCR_TB_SPRITES_HEIGHT;
	for (; tile_y < end; tile_y++, x += SCR_TB_SPRITES_WIDTH)
		SCR_TB_drawTile(screen, MAP_getTile(map, tile_x, tile_y), x);
}

void SCR_TB_drawTileRow(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y, u16 y)
{
	s16 end = tile_x + SCR_TB_PLANE_WIDTH;
	for (; tile_x < end; tile_x++, y++)
		SCR_TB_drawTile(screen, MAP_getTile(map, tile_x, tile_y), y);
}

void SCR_TB_drawAllTiles(struct SCR_Screen *screen, struct MAP_Map *map, MAP_Pos tile_x,
		MAP_Pos tile_y)
{
	for (u16 y = 0; tile_y < SCR_TB_SPRITES_HEIGHT; tile_y += 1, y += SCR_TB_SPRITES_WIDTH)
		SCR_TB_drawTileRow(screen, map, tile_x, tile_y, y);
}

// TODO: Implement this
void SCR_TB_updateAnimatedTiles(struct SCR_Screen *screen, struct MAP_Map *map,
		MAP_Pos tile_x, MAP_Pos tile_y)
{
	(void)screen;
	(void)map;
	(void)tile_x;
	(void)tile_y;
}
