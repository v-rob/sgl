// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#pragma once

#include "common.h"
#include "game.h"
#include "screen.h"

/* Fixed/integer types:
	* pos_t is a 24.8 fixed point position of an object. It represents an object's absolute position anywhere on the map.
	* vel_t is like pos_t, but only 8.8 fixed point. This should be used for a velocity or distance that is somewhere
	  around the screen size where a pos_t's precision would be overkill.
	* pos_t and vel_t are compatible with each other, so no fixed point conversion is necessary between them.
*/
TYPEDEF(s32, pos_t)
#define pos_t_POINT 8
TYPEDEF(s16, vel_t)
#define vel_t_POINT 8

// Object types
enum obj_type
{
	OBJ_GRAYFORD,
	OBJ_SUPER_GRAYFORD,
	OBJ_TESTER,
	OBJ_LEN
};

// Enum describing the position of where one object collided with another.
// Values can be combined to describe multiple directions, e.g. 'HIT_TOP | HIT_LEFT' for a top-left collision.
// 'HIT_PRIO_*' are values that are combined with the base direction to describe which direction was primarily hit.
enum hit
{
	HIT_NONE   = 0,
	HIT_TOP    = 0x1,
	HIT_BOTTOM = 0x2,
	HIT_LEFT   = 0x4,
	HIT_RIGHT  = 0x8,

	HIT_VERT  = HIT_TOP | HIT_BOTTOM,
	HIT_HORIZ = HIT_LEFT | HIT_RIGHT,
	HIT_ALL   = HIT_VERT | HIT_HORIZ,

	HIT_PRIO_NONE   = 0,
	HIT_PRIO_TOP    = 0x10,
	HIT_PRIO_BOTTOM = 0x20,
	HIT_PRIO_LEFT   = 0x40,
	HIT_PRIO_RIGHT  = 0x80
};

// An actual dynamic in-game object. Keep small.
// An object's internal position is the top-left corner of the rect. Although this means that the object has to be moved
// around after it is placed on the map to make its bottom be on the floor, it greatly simplifies collisions and drawing.
struct obj
{
	// 20 bytes

	// Current position and velocity of the object. Note that whenever an object is moving (not including things like teleportation),
	// the velocity should always be changed without adding directly to the position because collision detection requires the
	// velocity to find the old position.
	// NEVER change the X position manually without using the TODO member function. It messes up the object linked list organization!
	v2_pos_t pos;
	v2_vel_t vel;

	// Object linked list indicies for the objects to the right and left. If there is no object in that direction (i.e. this is the
	// leftmost or rightmost object), the value is NULL_INDEX.
	u16 right;
	u16 left;

	// Object's type, referring to an object definition.
	PACKED_ENUM(enum obj_type, u8) type;

	// Auxillary variables that are free for the callback functions to use at will.
	u8 aux_1;
	u8 aux_2;
	bool aux_3: 1;

	// Offset from the base sprite position to show. Mainly for animations or different object states.
	u8 sprite_offset: 3;
	// Delay between increments of the animation, to be used with 'step_animation'. If animations are unused, this is free to use.
	u8 ani_delay: 2;
	// Whether the sprite should be flipped in the X and Y directions
	bool flip_x: 1;
	bool flip_y: 1;
};

// A static definition for an object.
struct obj_def
{
	// Called when an object is created from a stored map object. Position, type, etc. are already filled in from the
	// map object, so this function is just for extra things that must be done, e.g. set the aux variables. NULL if
	// no custom constructor is necessary.
	void (*construct)(struct obj *obj);

	// Called every frame. This is where everything about the object is handled, like collisions, movement, and any
	// special features of the object. NULL means that there is no callback and the object is basically static.
	void (*update)(struct obj *obj);

	// Called when this object collides with another. Note that this callback will also be called for the other object
	// as well. NULL if nothing happens on collision.
	void (*collide)(struct obj *obj, enum hit hit, struct obj *other);

	// Size (width and height) and offset from the top-left of the object's internal position for the collision box
	v2_pixel_t rect_size;
	v2_pixel_t rect_offset;

	// Position of the object's base sprite on the spritesheet. The current animation offset is stored in the object itself.
	v2_sprite_t sprite_pos;
	// Extra tile width and height for objects that are taller and/or wider than one tile. 0 means that the object is only 1 wide/tall.
	v2_sprite_t extra_sprite;
	// True if further animation frames are stacked vertically instead of horizontally.
	bool ani_vert: 1;

	// True if not to collide with tiles/other objects
	bool no_tile_collision: 1;
	bool no_obj_collision: 1;
	// True if the top of the object is solid; used for standable platforms. Acts like tile cloud collision.
	bool solid_top: 1;
};

// Constant object definitions
extern const struct obj_def OBJ_DEFS[OBJ_LEN];

// Get an object definition
static inline const struct obj_def *get_obj_def(enum obj_type type)
{
	return &OBJ_DEFS[type];
}

// Make the object wrap around the level if it has a negative position
void wrap_obj(struct obj *obj);

// Get the center point of an object's rect
v2_pixel_t get_rect_center(const struct obj_def *def);

// Step an object's animation specifying how many frames to delay and the highest frame
void step_animation(struct obj *obj, u8 extra_delay, u8 highest_frame_offset);

// Update all objects: Modifying active object range, updating objects, moving them, collision detection, etc.
void update_objects(void);

// Check for object-tile collision and move the object out of collision.
// Returns the direction(s) of the tile(s) collided with, e.g. 'HIT_TOP | HIT_LEFT' means there was a ceiling and left wall
// collision. Priority hit values are unused.
/* Known inaccuracies:
	* Objects may not move at a velocity above one tile/frame; if they do, they might clip through tiles. This is pretty
	  good for a corrective collision detection algorithm, though.
	* Objects with a rect wider than one tile or taller than two tiles may not collide properly. Also, rects less than
	  four pixels (offset + width) will collide badly with walls if the object turns around while it is touching.
	* Objects moving at fast speeds may clip around outer corners, but it only happens at extremely high speeds with barely
	  noticeable results.
*/
enum hit tile_collision(struct obj *obj);

// Check for object-object collision and push the object out of each other if solidity is encountered.
/* Returns the ORed hit direction(s) and priority hit direction:
	* The ORed 'HIT_*' return value(s) describe the sides of the second object that the first is hitting, e.g. 'HIT_TOP | HIT_LEFT'
	  means that the first object is moving into the top and left sides of the second.
		* If no direction could be determined in a single dimension, no value is ORed for that dimension, e.g. if both
		  objects are not moving horizontally at all, the return value will have neither 'HIT_LEFT' nor 'HIT_RIGHT'.
		* A collision of e.g. 'HIT_TOP' does not imply that the first was above the second before this collision. The first
		  might have already been in the second object before this frame, and therefore didn't actually hit the top of the rect.
		  All the same, a 'HIT_TOP' value is returned to show that this is the side that is being hit.
	* The single or blank 'HIT_PRIO_*' return value describes the primary side that the first object is hitting. It is
	  essentially the same as the 'HIT_*' values except that it only gives a single value which is the most prominant.
		* For instance, a 'HIT_PRIO_TOP' shows that the first object mainly stomped or landed on top of the second. In the same
		  example, the hit values might have been 'HIT_TOP | HIT_LEFT', giving the complete story that it was mainly a stomp with
		  a smaller move to the left as well.
		* If two 'HIT_*' values are equivalent in strength, the priority will favor the X direction over the Y. The rationale
		  is that if the character lands on an enemy at a perfect 45 degree angle, it will probably be more likely that the
		  enemy would hurt the player. This circumstance is rare, so it likely doesn't matter much.
		* A priority hit can have a directional value when the corresponding hit doesn't have anything in that dimension, e.g.
		  'HIT_PRIO_LEFT | HIT_TOP'. This means that both object are moving in that dimension at the same speed. So, in the previous
		  example, both objects are moving left at the same speed, but that left motion is larger than the vertical motion.
		* A priority hit can also have no value. This will only happen if both objects are not moving at all but are touching.
	Got all that? There are a lot of edge cases. If you're confused, look at some object code.
*/
enum hit obj_collision(struct obj *first, struct obj *second);
