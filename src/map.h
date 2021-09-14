// Super Grayland: Copyright 2021 Vincent Robinson under the zlib license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#pragma once

#include "common.h"

// MAP Namespace: Everything related to the static map, specifically tiles and specials.
/*
	Super Grayland has a great tile system. It allows for two sprites to be layered on top of
	each other. Each tile can have different collision and physics properties without them
	having to be hardcoded for each sprite. Also, each tile can have a custom action tied
	to them which can do all sorts of nifty things.

	However, each tile is a full six bytes in memory, which would add up to a lot when multiplied
	by the level width and height. So, Super Grayland stores all of these tile definitions in
	a separate array and stores 8-bit indices into this array in the actual two-dimensional
	level array. This limits the number of unique tile definitions to 255 per level (the reserved
	256th one is for file compression usage); however, this is plenty for most usages, and
	level change specials can be used if more is necessary.
*/

// Position of a tile or dimensions of a set of tiles on the map.
typedef s16 MAP_Pos;
#define MAP_Pos_POINT 0

// The scrolled position of the map in pixels.
typedef s32 MAP_Scroll;
#define MAP_Scroll_POINT 3

// Defines how physical objects will interact with a tile.
enum MAP_Collision {
	MAP_Collision_AIR,   // Nothing will not collide with this tile
	MAP_Collision_CLOUD, // Solid from the top, but possible to jump/walk through the bottom/sides.
	MAP_Collision_SOLID  // Completely solid tile.
};

// Defines special physics or other property for a tile.
// If the property only works for certain collision types, those types are documented.
enum MAP_Property {
	MAP_Property_NORMAL,    // The tile does nothing special. This is default.
	MAP_Property_HAZARD,    // Player is hurt when touching the tile.
	MAP_Property_DEATH,     // Player dies upon touching the tile.
	MAP_Property_SWIMMABLE, // Player swims in the tile; object gravity is lower. Only air/cloud.
	MAP_Property_CLIMBABLE, // Player can climb the tile. Only air/cloud.
	MAP_Property_SLIPPERY,  // Acceleration is decreased when inside/on top of the tile.
	MAP_Property_STICKY,    // Max velocity/jump is decreased when inside/on top of the tile.
	MAP_Property_BOUNCY     // Tile reverses velocity on contact. Cloud, solid.
};

// TODO: Find a better built-in special system
enum MAP_BuiltInSpecial {
	MAP_BuiltInSpecial_COIN,
	MAP_BuiltInSpecial_BRICK,
	MAP_BuiltInSpecial_COIN_BLOCK,
	MAP_BuiltInSpecial_MULTI_COIN_BLOCK,
	MAP_BuiltInSpecial_CLOVER_BLOCK,
	MAP_BuiltInSpecial_BOOMERANG_BLOCK,
	MAP_BuiltInSpecial_TOP_BLOCK,
	MAP_BuiltInSpecial_HEART_BLOCK,
	MAP_BuiltInSpecial_MOON_BLOCK,
	MAP_BuiltInSpecial_LAMPPOST
};

// A definition of a tile. This struct should be kept smallish to ensure tile definitions don't
// take too much memory.
struct MAP_TileDef
{
	// 6 bytes

	// Sprite offsets for the back/front tile
	u16 Back;
	u16 Front;

	// The special action, which can optionally be one of MAP_Special's values
	u8 Special; /* can be enum MAP_Special */

	// The collision action of the tile.
	u8 Collision: 2; /* enum MAP_Collision */
	// The extra physics/property of the tile.
	u8 Property: 3;  /* enum MAP_Property */

	u8 : 3; // Extra space
};

// An index into one of the 255 tile definitions.
typedef u8 MAP_TileIndex;

// Defines how a level should wrap.
enum MAP_Wrap
{
	MAP_Wrap_NONE, // The map will not wrap. This is default.
	MAP_Wrap_OBJS, // Objects passing across one side of the map will wrap to the other side.
	MAP_Wrap_LEVEL // The entire level scrolls continuously. There are no visible edges.
};

struct MAP_Map
{
	// Array of tile definitions that is indexed into with `Indices`.
	struct MAP_TileDef *Defs;

	// Two-dimensional array of indices indexing into `Defs`. It is stored in rows from the top
	// left of the map, i.e. `map->Indices[map->SizeX]` is one tile below the top left tile.
	MAP_TileIndex *Indices;

	// The dimensions of the map in tiles.
	MAP_Pos SizeX;
	MAP_Pos SizeY;

	// The currently scrolled position of the map in pixels.
	MAP_Scroll ScrollX;
	MAP_Scroll ScrollY;

	// If and how to wrap the level.
	enum MAP_Wrap WrapX;
	enum MAP_Wrap WrapY;
};

// TODO: Const-ify all functions taking a struct, ensure void function(void) functions
// Initializes the map.
void MAP_init(struct MAP_Map *map);
// Deinitializes the map.
void MAP_deInit(struct MAP_Map *map);

/* Get a tile definition on the map.
	If the position is outside of the map boundaries, the behaviour varies:
	* For non-wrapping levels, it returns a solid tile if to the left or right of the map, an
	  air tile if above the map, and a death air tile if below the map.
	* For wrapping levels, it returns the wrap around tile, but will only wrap once. For
	  instance, MAP_getTile(map, 0, map->SizeX * 2) will return an incorrect tile and should
	  not be done.
*/
const struct MAP_TileDef *MAP_getTile(const struct MAP_Map *map, MAP_Pos pos_x, MAP_Pos pos_y);
