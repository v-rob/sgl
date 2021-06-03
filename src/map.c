// Super Grayland: Copyright 2021 Vincent Robinson under the MIT license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#include "map.h"

// Load a temporary test map
struct MAP_TileDef TempDefs[4] = {
	{0, 0, 0, MAP_Collision_AIR, MAP_Property_NORMAL},
	{3, 1, 0, MAP_Collision_SOLID, MAP_Property_NORMAL},
	{2, 0, 0, MAP_Collision_AIR, MAP_Property_NORMAL},
	{1, 0, 0, MAP_Collision_CLOUD, MAP_Property_NORMAL}
};

#define _ 0
#define x 1
#define t 2
#define o 3

MAP_TileIndex TempIndices[40 * 20] = {
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,x,_,x,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,o,o,o,o,o,o,o,o,x,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,t,_,x,x,_,x,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,x,_,_,_,_,_,_,_,t,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,x,_,_,_,_,_,_,_,t,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,x,x,x,x,x,x,x,x,x,x,x,x,x,o,x,x,x,x,x,x,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,x,x,x,x,_,_,_,_,_,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,o,_,_,_,_,_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,x,x,x,x,x,x,x,x,x,x,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,_,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,_,_,x,
	_,_,_,_,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,_,_,x,
	x,x,x,x,x,_,_,_,_,_,_,_,_,_,_,_,_,t,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,x,x,x,x,
};

void MAP_init(struct MAP_Map *map)
{
	map->Defs = TempDefs;
	map->Indices = TempIndices;

	map->SizeX = 40;
	map->SizeY = 20;

	map->ScrollX = 0;
	map->ScrollY = 0;

	map->WrapX = MAP_Wrap_NONE;
	map->WrapY = MAP_Wrap_NONE;
}

void MAP_deInit(struct MAP_Map *map)
{
	COM_zero(map);
}

// Generic tiles for tile collision off the level boundaries
const struct MAP_TileDef PlainSolid = {.Collision = MAP_Collision_SOLID};
const struct MAP_TileDef PlainAir   = {.Collision = MAP_Collision_AIR};
const struct MAP_TileDef DeathAir   = {.Collision = MAP_Collision_AIR, .Property = MAP_Property_DEATH};

const struct MAP_TileDef *MAP_getTile(struct MAP_Map *map, MAP_Pos pos_x, MAP_Pos pos_y)
{
	// Handle objects/anything else going out of the level boundaries
	if (pos_x < 0 || pos_x >= map->SizeX) {
		if (map->WrapX) {
			// Modfiy position to get the wrap-around tile if the level wraps
			if (pos_x < 0)
				pos_x = pos_x + map->SizeX;
			else
				pos_x = pos_x - map->SizeX;
		} else {
			return &PlainSolid; // Stop the player at the edge of the level
		}
	}

	if (pos_y < 0 || pos_y >= map->SizeY) {
		if (map->WrapY) {
			if (pos_y < 0)
				pos_y = pos_y + map->SizeY;
			else
				pos_y = pos_y - map->SizeY;
		} else {
			if (pos_y < 0)
				return &PlainAir; // Allow the player to jump above the level
			else
				return &DeathAir; // Kill the player when falling off the edge
		}
	}

	return &map->Defs[map->Indices[pos_y * map->SizeX + pos_x]];
}
