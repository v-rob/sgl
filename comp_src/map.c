// Super Grayland, Copyright (t) 2020 Matthew Robinson under the MIT license. See 'LICENSE.txt' for more information.

#include "map.h"
#include "obj.h"
#include "player.h"

struct map g_map;

bool init_map(void)
{
	g_game.state = STATE_LEVEL;

	// Load ze mapz
	struct tile _ = {TILE_AIR, TILE_AIR, SPECIAL_NONE, SOLIDITY_AIR, PROPERTY_NORMAL, false, false, false};
	struct tile x = {TILE_X, TILE_BLOCK, SPECIAL_NONE, SOLIDITY_SOLID, PROPERTY_NORMAL, false, true, false};
	struct tile t = {TILE_CROSS, TILE_AIR, SPECIAL_NONE, SOLIDITY_AIR, PROPERTY_NORMAL, false, false, false};
	struct tile o = {TILE_BLOCK, TILE_AIR, SPECIAL_NONE, SOLIDITY_CLOUD, PROPERTY_NORMAL, true, false, false};

	struct tile map[40 * 20] = {
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

	g_map.tiles = malloc(sizeof(struct tile) * 40 * 20);
	if (g_map.tiles == NULL) {
		ERROR(ALLOC, "map");
		return FAILURE;
	}

	memcpy(g_map.tiles, map, sizeof(struct tile) * 40 * 20);

	g_map.size = (v2_tile_t) {40, 20};
	g_map.scroll = (v2_scroll_t) {0, 0};

	g_map.special_flags = 0;

	g_map.wrap_horiz = WRAP_NONE;
	g_map.wrap_vert  = WRAP_NONE;

	// Zen we loadz ze objectz
	struct obj player = {
		.left = NULL_INDEX,
		.right = 1,
		.type = OBJ_GRAYFORD,
		.pos = {Frac_NEW(pos_t, 6), Frac_NEW(pos_t, 8)},
	};
	struct obj bob_ross = {
		.left = 0,
		.right = NULL_INDEX,
		.type = OBJ_TESTER,
		.pos = {Frac_NEW(pos_t, 8), Frac_NEW(pos_t, 8)},
	};

	g_map.objects = malloc(sizeof(struct obj) * 2);
	if (g_map.objects == NULL) {
		ERROR(ALLOC, "objects");
		return FAILURE;
	}
	g_map.objects[0] = player;
	g_map.objects[1] = bob_ross;

	g_map.num_objects = 2;
	g_map.reserved_obj_space = 2;

	g_map.player_index = 0;
	g_map.left_index = 0;
	g_map.right_index = 1;
	g_map.left_active_index = 0;
	g_map.right_active_index = 1;

	return SUCCESS;
}

void deinit_map(void)
{
	// Free the object space completely. Malloc will always be used to reinitialize instead of realloc to prevent heap fragmentation
	free(g_map.tiles);
	free(g_map.objects);
	free(g_map.player);
}

// Generic tiles for tile collision off the level boundaries
const struct tile PLAIN_AIR   = {.solidity = SOLIDITY_AIR};
const struct tile PLAIN_SOLID = {.solidity = SOLIDITY_SOLID};

const struct tile *get_tile(tile_t pos_x, tile_t pos_y)
{
	// Handle objects/anything else going out of the level boundaries
	if (pos_x < 0 || pos_x >= g_map.size.x) {
		if (g_map.wrap_horiz) {
			// Modfiy position to get the wrap-around tile if the level wraps
			if (pos_x < 0)
				pos_x = pos_x + g_map.size.x;
			else
				pos_x = pos_x - g_map.size.x;
		} else {
			// Return a generic solid tile to stop the player at the sides
			return &PLAIN_SOLID;
		}
	}

	if (pos_y < 0 || pos_y >= g_map.size.y) {
		if (g_map.wrap_vert) {
			if (pos_y < 0)
				pos_y = pos_y + g_map.size.y;
			else
				pos_y = pos_y - g_map.size.y;
		} else {
			// Return a generic air tile to let the player jump above the level and fall off the cliff
			return &PLAIN_AIR;
		}
	}

	return &g_map.tiles[pos_y * g_map.size.x + pos_x];
}

struct obj *get_right_obj(const struct obj *obj, u16 after)
{
	if (obj->right != after && obj->right != NULL_INDEX)
		return get_obj(obj->right);
	return NULL;
}

struct obj *get_left_obj(const struct obj *obj, u16 after)
{
	if (obj->left != after && obj->left != NULL_INDEX)
		return get_obj(obj->left);
	return NULL;
}

void move_obj_right_of(u16 obj, u16 other)
{
	// Get the necessary objects
	struct obj *o_obj   = get_obj(obj);
	struct obj *o_other = get_obj(other);

	// Fill in the space where 'obj' is
	if (o_obj->right != NULL_INDEX)
		get_obj(o_obj->right)->left = o_obj->left;
	if (o_obj->left  != NULL_INDEX)
		get_obj(o_obj->left)->right = o_obj->right;

	// Make 'obj' references point to the new position
	o_obj->right = o_other->right;
	o_obj->left = other;

	// Make the objects surrounding 'obj' point to it
	if (o_other->right != NULL_INDEX)
		get_obj(o_other->right)->left = obj;
	o_other->right = obj;
}

void move_obj_left_of(u16 obj, u16 other)
{
	struct obj *o_obj   = get_obj(obj);
	struct obj *o_other = get_obj(other);

	if (o_obj->right != NULL_INDEX)
		get_obj(o_obj->right)->left = o_obj->left;
	if (o_obj->left  != NULL_INDEX)
		get_obj(o_obj->left)->right = o_obj->right;

	o_obj->right = other;
	o_obj->left = o_other->left;

	if (o_other->left != NULL_INDEX)
		get_obj(o_other->left)->right = obj;
	o_other->left = obj;
}

struct obj *add_obj(pos_t pos_x)
{
	// Note that there will always be at least one object in the list, the player, so no checks need to be
	// made to see if the list is empty.

	// See if we can hold any more objects
	if (g_map.num_objects == 65535) {
		ERROR(ALLOC, "objects");
		return NULL;
	}
	g_map.num_objects++;

	// Increase the size to accommodate the extra object if necessary
	if (g_map.num_objects > g_map.reserved_obj_space) {
		g_map.reserved_obj_space++;
		g_map.objects = realloc(g_map.objects, sizeof(struct obj) * g_map.num_objects);
		if (g_map.objects == NULL) {
			ERROR(ALLOC, "objects");
			return NULL;
		}
	}

	// Create the object in the empty position after the end of the list
	u16 new_obj_index = g_map.num_objects - 1;
	struct obj *new_obj = get_obj(new_obj_index);

	// Initialize the object with everything zeroed out except for the X position
	*new_obj = (struct obj) {
		.pos.x = pos_x,
		.left  = NULL_INDEX,
		.right = NULL_INDEX
	};

	// Using the provided X position, find the correct sorted place for the object.
	// Start searching from the player position as the newly created object is likely to be near the player.
	struct obj *it = get_obj(g_map.player_index);
	if (pos_x >= it->pos.x) {
		// Advance iterator immediately as the player object doesn't need to be checked twice
		u16 it_index = it->right;
		it = get_right_obj(it, NULL_INDEX);

		for (; it != NULL; it_index = it->right, it = get_right_obj(it, NULL_INDEX)) {
			// If the iterator has passed the right of the object, insert the new object to the left of it
			if (it->pos.x > pos_x) {
				move_obj_left_of(new_obj_index, it_index);
				goto search_end;
			}
		}

		// No suitable position was found to the left of any objects, so insert the object at the very right
		move_obj_right_of(new_obj_index, it_index);
	} else {
		u16 it_index = it->left;
		it = get_left_obj(it, NULL_INDEX);

		for (; it != NULL; it_index = it->left, it = get_left_obj(it, NULL_INDEX)) {
			if (it->pos.x < pos_x) {
				move_obj_right_of(new_obj_index, it_index);
				goto search_end;
			}
		}

		move_obj_left_of(new_obj_index, it_index);
	}
search_end:

	return new_obj;
}

void remove_obj(u16 index)
{
	// Note that there will always be at least one object in the list, the player, so no checks need to be
	// made to see if the list is empty.

	// We don't realloc the object list space smaller because that might take extra time. We don't need to
	// conserve memory anyway as the object list is the only thing that will ever be realloced
	g_map.num_objects--;

	// TODO
}
