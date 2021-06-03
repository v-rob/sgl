// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#pragma once

#include "common.h"
#include "game.h"
#include "obj.h"
#include "screen.h"

/* Fixed/integer types:
	* tile_t is a 16 bit integer defining the absolute position of a tile. This should be used with anything that refers to
	  an absolute tile position instead of a raw integer type.
	* tile_t and sprite_t are compatible with each other, so no fixed point conversion is necessary between them.
*/
TYPEDEF(s16, tile_t)
#define tile_t_POINT 0

// Tiles; tile names and tile sprite indexes are the same
enum tile_type
{
	TILE_AIR,
	TILE_BLOCK,
	TILE_CROSS,
	TILE_X,
	TILE_LEN
};

// Solidity type of a tile
enum solidity
{
	SOLIDITY_AIR,
	SOLIDITY_CLOUD,
	SOLIDITY_SOLID
};

// Special action or physics for a tile
enum property
{
	PROPERTY_NORMAL,
	PROPERTY_HAZARD,
	PROPERTY_DEATH,
	PROPERTY_SWIMMABLE,
	PROPERTY_CLIMBABLE,
	PROPERTY_SLIPPERY,
	PROPERTY_STICKY,
	PROPERTY_BOUNCY
};

// ID of a special tile or a built-in special tile
// TODO: Place powerup inside block?
enum special_id
{
	SPECIAL_NONE = 0,
	SPECIAL_BEGIN_BUILT_IN = 246, // Marker for the built in special actions
	SPECIAL_BRICK = 246,
	SPECIAL_COIN_BRICK = 247,
	SPECIAL_COIN_BLOCK = 248,
	SPECIAL_MULTI_COIN_BLOCK = 249,
	SPECIAL_CLOVER_BLOCK = 250,
	SPECIAL_BOOMERANG_BLOCK = 251,
	SPECIAL_TOP_BLOCK = 252,
	SPECIAL_HEART_BLOCK = 253,
	SPECIAL_MOON_BLOCK = 254,
	SPECIAL_COIN = 255,
	// TODO: End of level
};

// A single tile on the map
// This struct _must_ be kept small since the level is a giant global array of these
struct tile
{
	// 6 bytes

	// Back and front layer tiles
	PACKED_ENUM(enum tile_type, u16) back;
	PACKED_ENUM(enum tile_type, u16) front;

	// Special tile index to use, optionally one of 'enum special_id'
	u8 special;

	// How the tile interacts with objects
	PACKED_ENUM(enum solidity, u8) solidity: 2;
	PACKED_ENUM(enum property, u8) property: 3;

	// Whether front and back layer tiles are in the foreground (in front of objects)
	bool is_back_fg: 1;
	bool is_front_fg: 1;

	// Whether the special has been activated or not TODO: Move to struct special
	bool special_activated: 1;
};

// How a special tile is activated
enum activation
{
	ACTIVATION_AUTO,
	ACTIVATION_UP,
	ACTIVATION_DOWN,
	ACTIVATION_LEFT,
	ACTIVATION_RIGHT
};

// What animation to use, if any, for a special tile
enum animation
{
	ANIMATION_NONE,
	ANIMATION_UP,
	ANIMATION_DOWN,
	ANIMATION_LEFT,
	ANIMATION_RIGHT,
	ANIMATION_DOOR
};

// Whether a special tile has text
enum text_type
{
	TEXT_TYPE_NONE,
	TEXT_TYPE_PLAYER,
	TEXT_TYPE_BOTTOM,
	TEXT_TYPE_SIGN
};

// How to change a special tile flag
enum flag_operation
{
	FLAG_OPERATION_NONE,
	FLAG_OPERATION_SET,
	FLAG_OPERATION_RESET,
	FLAG_OPERATION_TOGGLE
};

// A special tile definition. This struct should be kept semi-small.
// Note that this doesn't have very good organization due to the necessity of packing closely, but many parameters have
// corresponding flags that determine whether to use it or not and how.
struct special
{
	// 16 bytes
	// Teleport options
	v2_tile_t teleport_pos;
	// Changing something options
	u16 new_level;
	u8 new_music;
	// Flag operations
	u8 flag_to_get;
	u8 flag_to_set;
	// Text
	u8 text_index;
	u8 failed_text_index;
	// Coins
	u8 coins_to_take;
	// Activation
	PACKED_ENUM(enum activation, u8) activation: 3;
	bool no_reactivate: 1;
	u8 : 1;
	// Changing something options (continued)
	bool change_music: 1;
	bool change_level: 1;
	bool end_of_level: 1;
	// Flag operations (continued)
	PACKED_ENUM(enum flag_operation, u8) flag_get_operation: 2; // TOGGLE = random
	PACKED_ENUM(enum flag_operation, u8) flag_set_operation: 2;
	// Teleport options (continued)
	bool use_teleport: 1;
	bool relative_teleport: 1;
	bool seamless_teleport: 1; // If true, preserves velocity; if false, rounds position to nearest tile
	bool teleport_sound_flash: 1;
	// Animation options
	PACKED_ENUM(enum animation, u8) ani_before: 3;
	PACKED_ENUM(enum animation, u8) ani_after: 3;
	// Text (continued)
	PACKED_ENUM(enum text_type, u8) text_type: 2;
	PACKED_ENUM(enum text_type, u8) failed_text_type: 2;
	bool show_text_last: 1;
	bool use_text_prompt: 1;
	u8 : 5;
};

/* Special Tile Notes: (TODO: Move to the special handler)
	Special tiles have a very specific order they are executed in:
	* flag/reactivation/coins checked
	* If failed:
		* failed text
	* Else:
		* text - defaults to first. If text prompt and 'no', abort
		* 	take coins
		* before animation
		* level changes - changing level is next, allowing following changes to modify the _next_ level
		* flag changes - the new level will not have the old bits
		* music changes - happens before teleportation and text so they have the new music, but not during the animation
		* teleportation - custom position in new level
		* after animation
		* text - can happen last (show_text_last)
		* 	take coins
*/

// An object as stored in a map file. Keep small.
struct map_obj
{
	// 6 bytes
	v2_tile_t pos;
	u8 aux_1;
	u8 aux_2;
	bool flip_x: 1;
	bool flip_y: 1;
	bool aux_3: 1;
	u8 : 5;
	u8 : 8;
};

// If and how a level should be wrapped in one dimension
enum wrap
{
	WRAP_NONE,
	WRAP_OBJS,
	WRAP_LEVEL
};

// An object list index that points to nothing. Equivalent of a NULL pointer.
#define NULL_INDEX 65535

// The map and all data common to it
struct map
{
	// Currently loaded map tiles
	struct tile *tiles;
	// Size of the map
	v2_tile_t size;
	// Current scrolled position on the map
	v2_scroll_t scroll;
	// If and how to wrap the level in both dimensions
	enum wrap wrap_horiz;
	enum wrap wrap_vert;

	/* Linked list of objects in the level:
		* The data type is a linked list, but it is different than most. All the objects are in a continguous chuck of
		  malloc'ed memory but in no particular order. This minimizes heap fragmentation, but the linked list component means
		  that new objects can just be added to the end of the chunk of memory. On deleting an object, the last object in the
		  list is moved into the emptied position and the list pointers are updated.
		* The player is a special object. The player is kept in the list with the objects instead of being completely externalized
		  because the benefits of sorting also apply to it as well as ensuring that there is at least one object in the list at all
		  times, greatly simplifying the code.
		* The indicies are u16 because there can never be more than 65535 objects (not 65536 so NULL_INDEX has a u16 number) and
		  everything is in the same list, so it is cheaper than a pointer.
		* When anything talks about a slice or reigon, it doesn't mean a slice in memory, but a slice from one index to another
		  through obj.right/left including both the start and end indexes in the slice.
		* The linked list is always organized according to the object's X position in the level from left to right. This allows
		  for an active object slice, which can grow and shrink more easily than a fixed size list of active objects. The
		  downside is that newly created objects and objects that have moved must be sorted.
	*/
	struct obj *objects;
	// Full number of objects in the level
	u16 num_objects;
	// How much space is reserved for the objects
	u16 reserved_obj_space;
	// Index of the player object
	u16 player_index;
	// Index of the leftmost/rightmost objects in the object list. NOT the same as 0 and num_objects - 1; these can anywhere
	// in the list.
	u16 right_index;
	u16 left_index;
	// Active object slice; objects within this linked list slice, will be updated in 'update_objects'. Others, far enough off
	// screen, will be ignored to save processing power. The active range size is determined by ACTIVE_OBJ_RADIUS. Objects are
	// always active vertically within this slice as levels are usually much longer than they are tall and it is tricky to
	// sort the object list vertically as well as horizontally.
	u16 right_active_index;
	u16 left_active_index;

	// The player has extra information that objects do not have TODO: Not pointer
	struct player *player;

	// Array of all special tiles
	struct special *specials;
	u16 num_specials;
	// Special tile currently being run. This is necessary instead of execution in place because loading a new level
	// will destroy the old specials.
	struct special current_special;
	// Bit flags for special tiles
	u32 special_flags;
};

// The map version for compatibility with older maps
#define MAP_VERSION 0

// Radius in tiles from the player's position to update objects. In total, the active object range is two whole screens wide.
#define ACTIVE_OBJ_RADIUS SCREEN_TILES_X

// Initialize/deinitialize the map
bool init_map(void);
void deinit_map(void);

// The currently loaded map
extern struct map g_map;

// Get a specific tile on the map. If the position is out of the map boundaries, it returns a tile with solidity information
// to describe the collision outside of the map, specifically solid walls in the X direction and air tiles in the Y direction.
const struct tile *get_tile(tile_t pos_x, tile_t pos_y);

// Gets an object from the list of objects from an index. NULL_INDEX is invalid.
static inline struct obj *get_obj(u16 index)
{
	return &g_map.objects[index];
}

// Returns the object to the right/left in the object list. Returns NULL if the index to the right/left is NULL_INDEX or is the
// index at 'after' (which can also be NULL_INDEX to only stop at the very right/left).
struct obj *get_right_obj(const struct obj *obj, u16 after);
struct obj  *get_left_obj(const struct obj *obj, u16 after);

// Moves the references to an object in the linked list to the right/left of the object 'other'. 'obj' and 'other' must not be
// NULL_INDEX, but NULL_INDEX references in each object are valid. Used for sorting.
void move_obj_right_of(u16 obj, u16 other);
void move_obj_left_of (u16 obj, u16 other);

// Inserts a new object in the object list. 'pos_x' is the initial X position of the object, required to place the object in a
// sorted position. On success, returns the created object where the only initialized property is the X position; everything
// else is zeroed. Returns NULL if it could not be created.
struct obj *add_obj(pos_t pos_x);

// Removes an object from the object list and moves the top object into the vacated place.
void remove_obj(u16 index);

/*
All files use big endian numbers
Change level = show score tally, title card, and screen flash

DON'T FORGET TO RESET ALL STUFF ON LOADING A NEW MAP!

flexible struct map_format
{
	u16 version;
	u8 level_name_len; // Even number for padding purposes
	char level_name[level_name_len];
	u8 act;
	u8 music_index;
	u16 width;
	u16 height;
	bool wrap_horizontally;
	bool wrap_vertically;
	bool use_rain;
	u5 padding;
	struct tile map[width * height];
	map_obj_t player;
	u8 num_objs;
	map_obj_t objs[num_objs];
	u8 num_specials;
	special_t specials[num_specials];
	u8 text_indexes;
	char *text;
};

flexible struct level_pack_format
{
}

struct save_file
{
	// How to store completed levels? Something to do with level packs
	u16 current_level;
	u16 score;
	u8 lives;
};
*/
