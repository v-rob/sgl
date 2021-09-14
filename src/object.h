// Super Grayland: Copyright 2021 Vincent Robinson under the zlib license.
// See `license.txt` for more information.
// Before delving into the code, please read `readme_source.txt` to understand the basic design.

#pragma once

#include "common.h"

#include "map.h"
#include "screen.h"

// OBJ Namespace: Dynamic object management
/*
	Objects are dynamic things that can move about, collide with things, and do all sorts of
	nifty things. Players, enemies, and power-ups are all objects.

	Objects are split into two structs: `OBJ_Object` contains all information unique to a
	single object, such as position and velocity, while `OBJ_ObjectDef` contains information
	and callbacks that are common to all objects of that type, such as collison boxes. A list
	of all `OBJ_ObjectDef` are stored in a global array which is indexed into by `OBJ_Type`.
	Each `OBJ_Object` contains an `OBJ_Type` that shows what type it is and allows access to
	its `OBJ_ObjectDef`.

	Objects are positioned from the bottom left corner. Collision boxes and the object's sprite
	are positioned relative to this bottom left corner. This makes collision detection code
	sightly more unintuitive due to implicit fixed point flooring when converting to lower
	precisions, but it makes positioning the object everywhere else make more sense.
*/

// Enum defining every type of object
enum OBJ_Type
{
	OBJ_Type_GRAYFORD,
	OBJ_Type_SUPER_GRAYFORD,
	OBJ_Type_TESTER,
	OBJ_Type_LEN
};

// Defines the position of an object with ample precision for physics calculations.
typedef s32 OBJ_Pos;
#define OBJ_Pos_POINT 8

// Defines the velocity of an object. This exists to save space in the `OBJ_Object` struct
// before multiplying or dividing, this must be converted to `OBJ_Pos` or else the entire
// integer portion will be truncated.
typedef s16 OBJ_Vel;
#define OBJ_Vel_POINT 8

// All dynamic parts of an object. This struct should be kept smallish to ensure objects don't
// take too much memory.
struct OBJ_Object
{
	// 16 bytes

	// Current position of the object. NEVER change the position without using `OBJ_addPosX/Y`
	// or `OBJ_setPosX/Y`; that would break object linked list sorting.
	OBJ_Pos PosX;
	OBJ_Pos PosY;

	// Current velocity of the object. This may not exceed eight as that would break collision
	// detection and slow down screen scrolling.
	OBJ_Vel VelX;
	OBJ_Vel VelY;

	// Type of the object. Used to index into `ObjectDefs` for this object's corresponding
	// `OBJ_ObjectDef`.
	u8 Type;

	// Auxillary variables that are free for the callback functions to use.
	union {
		struct {
			u8 Free1;
			u8 Free2;
		};
		u16 Free12;
	};
	bool Free3: 1;

	// Offset from the base sprite position to show. Mainly for animations and/or different
	// object states.
	u8 SpriteOffset: 3;
	// A counter counting up to a time before the next animation frame should occur, to be used
	// with `OBJ_stepAnimation` in `update`. If animations are unused, this is a free variable.
	u8 AnimationDelayCount: 2;

	// Whether the sprite is flipped across a certain axis
	bool FlipAcrossX;
	bool FlipAcrossY;
};

enum OBJ_Hit
{
	OBJ_Hit_NONE   = 0,
	OBJ_Hit_TOP    = 0x1,
	OBJ_Hit_BOTTOM = 0x2,
	OBJ_Hit_LEFT   = 0x4,
	OBJ_Hit_RIGHT  = 0x8,

	OBJ_Hit_VERT  = OBJ_Hit_TOP | OBJ_Hit_BOTTOM,
	OBJ_Hit_HORIZ = OBJ_Hit_LEFT | OBJ_Hit_RIGHT,
	OBJ_Hit_ALL   = OBJ_Hit_VERT | OBJ_Hit_HORIZ,

	OBJ_Hit_PRIO_NONE   = 0,
	OBJ_Hit_PRIO_TOP    = 0x10,
	OBJ_Hit_PRIO_BOTTOM = 0x20,
	OBJ_Hit_PRIO_LEFT   = 0x40,
	OBJ_Hit_PRIO_RIGHT  = 0x80
};

struct OBJ_ObjectDef
{
	// Called when an object is created from a stored map object. Position, type, etc. are
	// already filled in from the map object, so this function is just for extra things that
	// must be done, e.g. set the aux variables. NULL if no custom constructor is necessary.
	void (*Construct)(struct OBJ_Object *obj);

	// Called every frame. This is where everything about the object is handled, like collisions,
	// movement, and any special features of the object. NULL means that there is no callback
	// and the object is basically static.
	void (*Update)(struct OBJ_Object *obj);

	// Called when this object collides with another. Note that this callback will also be called
	// for the other object as well. NULL if nothing happens on collision.
	void (*Collide)(struct OBJ_Object *obj, struct OBJ_Object *other, enum OBJ_Hit hit);

	// Collision box dimensions of the object, measured from the bottom left of the object's
	// internal position.
	SCR_Pixel RectX;
	SCR_Pixel RectY;
	SCR_Pixel RectWidth;
	SCR_Pixel RectHeight;

	// Position of the object's base sprite in the object sprite bank. This can be changed/
	// animated by up to seven sprites with `SpriteOffset` in `OBJ_Object`.
	u16 Sprite;

	// If the object is wider or taller than one sprite, these define how much wider or taller.
	// In other words, zero means that the sprite is only one wide or tall.
	SCR_Pixel ExtraWidth;
	SCR_Pixel ExtraHeight;

	// True if not to run collision detection code with tiles/other objects
	bool NoTileCollision: 1;
	bool NoObjectCollision: 1;

	// True if the top of the object is solid; used for standable platforms. Acts like tile
	// cloud collision.
	bool IsSolid: 1;
};

extern const struct OBJ_ObjectDef OBJECT_DEFS[OBJ_Type_LEN];

#define OBJ_getDef(obj_type) (&OBJECT_DEFS[obj_type])

// Wrap an object around the map if it is beyond the map boundaries and the level is wrappable.
void OBJ_wrap(struct OBJ_Object *obj, const struct MAP_Map *map);

// Get the center point of an object definition's collision box.
void OBJ_getBoxCenter(const struct OBJ_ObjectDef *def, SCR_Pixel *x_out, SCR_Pixel *y_out);

// Step an object's animation by specifying how many frames to delay and the highest frame it
// may step to before wrapping. Modifies `SpriteOffset` and `AnimationDelayCount`.
void OBJ_stepAnimation(struct OBJ_Object *obj, u8 delay_end, u8 highest_frame_offset);
