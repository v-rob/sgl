// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#include "map.h"
#include "obj.h"
#include "player.h"

#define DEF_TESTER {		\
	.sprite_pos = {0, 1},	\
	.rect_size   = {15, 4},	\
	.rect_offset = {1, 2},	\
	.extra_sprite.x = 1		\
}

const struct obj_def OBJ_DEFS[OBJ_LEN] = {
	DEF_GRAYFORD,
	DEF_SUPER_GRAYFORD,
	DEF_TESTER
};

void wrap_obj(struct obj *obj)
{
	if (g_map.wrap_horiz) {
		if (obj->pos.x < 0)
			obj->pos.x += Frac_CONVERT(tile_t, pos_t, g_map.size.x);
		else if (obj->pos.x > Frac_CONVERT(tile_t, pos_t, g_map.size.x))
			obj->pos.x -= Frac_CONVERT(tile_t, pos_t, g_map.size.x);
	}
	if (g_map.wrap_vert) {
		if (obj->pos.y < 0)
			obj->pos.y += Frac_CONVERT(tile_t, pos_t, g_map.size.y);
		else if (obj->pos.y > Frac_CONVERT(tile_t, pos_t, g_map.size.y))
			obj->pos.y -= Frac_CONVERT(tile_t, pos_t, g_map.size.y);
	}
}

void step_animation(struct obj *obj, u8 extra_delay, u8 highest_frame_offset)
{
	WRAP_ADD(obj->ani_delay, +1, extra_delay, 0);

	if (obj->ani_delay != 0)
		return;

	WRAP_ADD(obj->sprite_offset, +1, highest_frame_offset, 0);
}

// TODO: Make all code use this instead of calculating it themselves
v2_pixel_t get_rect_center(const struct obj_def *def)
{
	return (v2_pixel_t) {
		def->rect_size.x / 2 + def->rect_offset.x,
		def->rect_size.y / 2 + def->rect_offset.y
	};
}

void update_objects(void)
{
	// TODO: Updating objects
}

enum hit tile_collision(struct obj *obj)
{
	// Collision detection; check if the object is inside a tile and bump out properly if so

	// If struggling to understand the code, try using an image editor and move a sprite around with pixels
	// representing corners or the object position. Also, mind the fixed point types!

	const struct obj_def *def = get_obj_def(obj->type);

	// Offsets must be flipped if the object is flipped
	// TODO: No flipping rect?
	v2_pixel_t offset = def->rect_offset;

	if (obj->flip_x || obj->flip_y) {
		if (obj->flip_x) {
			pixel_t sprite_size_x = Frac_CONVERT(sprite_t, pixel_t, def->extra_sprite.x + 1);
			offset.x = sprite_size_x - offset.x - def->rect_size.x;
		}
		if (obj->flip_y) {
			pixel_t sprite_size_y = Frac_CONVERT(sprite_t, pixel_t, def->extra_sprite.y + 1);
			offset.y = sprite_size_y - offset.y - def->rect_size.y;
		}
	}

	// Object sides which can be rounded down to the appropriate tile
	vel_t top = Frac_CONVERT(pixel_t, vel_t, offset.y);
	vel_t height = Frac_CONVERT(pixel_t, vel_t, def->rect_size.y) - 1; // Subtraction by one on right and bottom sides prevents
																	  // false-positive collisions since get_tile gets the
																	  // floored position, so minus one stays in the proper tile
	vel_t bottom = top + height;
	vel_t left = Frac_CONVERT(pixel_t, vel_t, offset.x);
	vel_t right = left + Frac_CONVERT(pixel_t, vel_t, def->rect_size.x) - 1;

	// true if the height of the rect is greater than one tile
	bool is_tall = offset.y + def->rect_size.y > 8;

	// Get a tile from a point on an object
#define GET_TILE(obj_x, obj_y)	\
		get_tile(Frac_CONVERT(pos_t, tile_t, obj->pos.x + (obj_x)), Frac_CONVERT(pos_t, tile_t, obj->pos.y + (obj_y)))->solidity

	/* Wall pushouts:
		* For both pushouts, the pushout must be done at the edge of the _rect_, not the edge of the _object_, so that the actual
		  rect is pushed out by CEIL or FLOOR. In the left pushout (the simpler one), 'left' is added inside CEIL and subtracted
		  again outside of it to achieve this. The right pushout is the same, but _everything_ must be reversed, complicating it.
		* Both collisions either add or subtract one somewhere. This is corrective to allow a full tile per frame to be achieved.
		  Why it is necessary is tricky to explain, and is only partially known by the author himself.
	*/
#define WALL_PUSHOUT()								\
	if (obj->vel.x < 0) { /* Left  */				\
		obj->pos.x = Frac_CEIL(pos_t, obj->pos.x + 1 + left) - left; \
	} else {			  /* Right */				\
		s16 beside = Frac_NEW(vel_t, 1) - right;	\
		obj->pos.x = Frac_FLOOR(pos_t, obj->pos.x + Frac_NEW(vel_t, 1) - beside) - Frac_NEW(vel_t, 1) + beside - 1; \
	}												\
	obj->vel.x = 0;

// Ceiling collision only requires checking the top two points for collision
#define CEILING_COLLISION(y_pos) (GET_TILE(left, y_pos) == SOLIDITY_SOLID || GET_TILE(right, y_pos) == SOLIDITY_SOLID)

	// TODO: Helper functions; no more macro laziness
	// For floor collision, check the bottom two points for collision. If it is a cloud collision, check if the old position is
	// above the tile being collided with; if so, register the collision since the player is not jumping through it from the bottom
	// Note: This macro is used in if statements and includes a variable assignment, so the variable is declared here
	enum solidity floor_hit;
#define FLOOR_COLLISION(y_pos) ((floor_hit = max(GET_TILE(left, y_pos), GET_TILE(right, y_pos))) && (floor_hit == SOLIDITY_SOLID ||		\
		(floor_hit == SOLIDITY_CLOUD && Frac_FLOOR(pos_t, obj->pos.y + bottom) > Frac_FLOOR(pos_t, obj->pos.y + bottom - obj->vel.y))))

	// Direction(s) the object collided in
	enum hit dir = HIT_NONE;

	// Check walls only if the object is moving in the direction of the wall
	vel_t x_coord = 0;
	if (obj->vel.x > 0) {
		x_coord = right;
		dir |= HIT_RIGHT;
	} else if (obj->vel.x < 0) {
		x_coord = left;
		dir |= HIT_LEFT;
	}

	if (x_coord) {
		bool hit = false;

		if (obj->vel.y == 0) {
			// Forward collision only needs to check the top, bottom, and, if tall, one tile above bottom points
			hit = GET_TILE(x_coord, top) == SOLIDITY_SOLID || GET_TILE(x_coord, bottom) == SOLIDITY_SOLID ||
					(is_tall && GET_TILE(x_coord, bottom - Frac_NEW(vel_t, 1)) == SOLIDITY_SOLID);
		} else if (obj->vel.y > 0) {
			/* Downward collision:
				* Check top point if velocity does not exceed the height so that it does not register a false collision
				  if moving really fast.
				* Check variable (bottom-ish) point; this moves up the side of the object opposite the vertical velocity. The
				  point of this is so no wall collision is registered when the object goes into the floor, but a collision
				  is registered if the object actually goes into the wall. Checking floor collision first instead would not
				  help because that would just register a different false positive for floors instead of walls. Even if the
				  point moves above the height of the rect (say a velocity of 7 and a height of 6), still check the point so
				  that _some_ wall collision is registered, even if it is slightly inaccurate.
				* If tall, always check one tile above bottom since this will always be above the floor.
			*/
			hit = GET_TILE(x_coord, bottom - obj->vel.y) == SOLIDITY_SOLID ||
					(is_tall && GET_TILE(x_coord, bottom - Frac_NEW(vel_t, 1)) == SOLIDITY_SOLID);

			if (obj->vel.y < height)
				hit = hit || GET_TILE(x_coord, top) == SOLIDITY_SOLID;
		} else {
			// Upward collision. See downward collision, but reversed
			hit = GET_TILE(x_coord, top - obj->vel.y) == SOLIDITY_SOLID ||
					(is_tall && GET_TILE(x_coord, top + Frac_NEW(vel_t, 1)) == SOLIDITY_SOLID);

			if (-obj->vel.y < height)
				hit = hit || GET_TILE(x_coord, bottom) == SOLIDITY_SOLID;
		}

		if (hit) {
			WALL_PUSHOUT()
		}
	}

	// Double tall objects can move into a one tile high area, so check it is caught.
	if (is_tall && ((FLOOR_COLLISION(bottom) && CEILING_COLLISION(bottom - Frac_NEW(vel_t, 2))) ||
			(FLOOR_COLLISION(top + Frac_NEW(vel_t, 2)) && CEILING_COLLISION(top))))
	{
		// Push the object back towards whence it came
		WALL_PUSHOUT()
	}

	// Floor and ceiling collisions here are just like wall collisions
	if (obj->vel.y > 0 && FLOOR_COLLISION(bottom)) {
		obj->vel.y = 0;

		// See the right pushout behaviour for an explanation of floor pushout behaviour
		s16 extra_height = Frac_CONVERT(tile_t, vel_t, is_tall);
		s16 below = extra_height - bottom;
		obj->pos.y = Frac_FLOOR(pos_t, obj->pos.y + extra_height - below) - extra_height + below - 1;

		dir |= HIT_BOTTOM;
	} else if (obj->vel.y < 0 && CEILING_COLLISION(top)) {
		obj->vel.y = 0;

		// See the left pushout behaviour for an explanation of ceiling pushout behaviour
		obj->pos.y = Frac_CEIL(pos_t, obj->pos.y + 1 + top) - top;

		dir |= HIT_TOP;
	}

#undef GET_TILE
#undef WALL_PUSHOUT

	return dir;
}

// Get the hit direction the object-object collision was in
static enum hit get_hit_dir(vel_t first, vel_t second, enum hit top_left, enum hit bottom_right)
{
	if (first > 0) {
		if (second > 0) {
			// Both are moving in the same direction. Find the faster object to determine which side is hit
			if (first > second)
				// The first overtook the second, so it hit the second's back
				return top_left;
			else if (second > first)
				// The second just overtook the first, so the first hit the front of the second
				return bottom_right;
			else
				// Both have the same speed
				return HIT_NONE;
		} else {
			// The second is not moving or is moving opposite of the first
			return top_left;
		}
	} else if (first < 0) {
		// Same as above, but in the opposite direction
		if (second < 0) {
			if (first < second)
				return bottom_right;
			else if (second < first)
				return top_left;
			else
				return HIT_NONE;
		} else {
			return bottom_right;
		}
	} else {
		// The first is not moving, so check the second's movement for collision
		if (second > 0)
			return bottom_right;
		else if (second < 0)
			return top_left;
		else
			// Neither are moving
			return HIT_NONE;
	}
}

enum hit obj_collision(struct obj *first, struct obj *second)
{
	// Object collision detection is simple AABB collision with velocity checks to determine collision direction
	// TODO: Pushout for solid objects

	const struct obj_def *first_def  = get_obj_def(first->type);
	const struct obj_def *second_def = get_obj_def(second->type);

	// Upper left corners
	v2_pos_t first_ul = {
		Frac_CONVERT(pixel_t, pos_t, first_def->rect_offset.x) + first->pos.x,
		Frac_CONVERT(pixel_t, pos_t, first_def->rect_offset.y) + first->pos.y
	};
	v2_pos_t second_ul = {
		Frac_CONVERT(pixel_t, pos_t, second_def->rect_offset.x) + second->pos.x,
		Frac_CONVERT(pixel_t, pos_t, second_def->rect_offset.y) + second->pos.y
	};

	// Lower right corners
	v2_pos_t first_lr = {
		Frac_CONVERT(pixel_t, pos_t, first_def->rect_size.x) + first_ul.x,
		Frac_CONVERT(pixel_t, pos_t, first_def->rect_size.y) + first_ul.y
	};
	v2_pos_t second_lr = {
		Frac_CONVERT(pixel_t, pos_t, second_def->rect_size.x) + second_ul.x,
		Frac_CONVERT(pixel_t, pos_t, second_def->rect_size.y) + second_ul.y
	};

	enum hit dir = HIT_NONE;

	// Check for collision with AABB collision detection
	if (first_ul.x < second_lr.x && first_lr.x > second_ul.x &&
		first_ul.y < second_lr.y && first_lr.y > second_ul.y)
	{
		// Find the direction the collision was in ('HIT_*' values)
		enum hit vert  = get_hit_dir(first->vel.y, second->vel.y, HIT_TOP, HIT_BOTTOM);
		enum hit horiz = get_hit_dir(first->vel.x, second->vel.x, HIT_LEFT, HIT_RIGHT);

		dir = vert | horiz;

		// Find the strongest velocity and therefore the priority direction
		if (first->vel.x == 0 && first->vel.y == 0 && second->vel.x == 0 && second->vel.y == 0)
			// Both objects are at a full stop; no priority can be determined
			return dir;

		// Find the larger delta of each dimension to find the stronger velocity
		if (abs(first->vel.x - second->vel.x) >= abs(first->vel.y - second->vel.y)) {
			// X is stronger, so set the priority based on the horiz hit parameter
			if (horiz == HIT_LEFT) {
				dir |= HIT_PRIO_LEFT;
			} else if (horiz == HIT_RIGHT) {
				dir |= HIT_PRIO_RIGHT;
			} else {
				// The objects are moving at the same speed, so get the priority from the object's velocity
				if (first->vel.x > 0)
					dir |= HIT_PRIO_LEFT;
				else
					dir |= HIT_PRIO_RIGHT;
			}
		} else {
			// Y is stronger. Do the same as X, but in the vertical direction
			if (vert == HIT_TOP) {
				dir |= HIT_PRIO_TOP;
			} else if (vert == HIT_BOTTOM) {
				dir |= HIT_PRIO_BOTTOM;
			} else {
				if (first->vel.y > 0)
					dir |= HIT_PRIO_TOP;
				else
					dir |= HIT_PRIO_BOTTOM;
			}
		}
	}

	return dir;
}
