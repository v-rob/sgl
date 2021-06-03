// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#include "map.h"
#include "player.h"

void update_player(const u8 *keys)
{
	// Handle the key presses
	g_map.player->obj.vel = (v2_vel_t) {0, 0};
	vel_t speed = Frac_NEW_FRAC(vel_t, 0, 1, 8);

	// TODO: Change key detection order?
	// Pause
	if (keys[SDL_SCANCODE_ESCAPE]) {
		g_game.state = STATE_QUIT;
		return;
	}
	// Run
	if (keys[SDL_SCANCODE_A]) {
		speed = Frac_CONVERT(scroll_t, vel_t, 8);
	}
	// Move left
	if (keys[SDL_SCANCODE_LEFT]) {
		g_map.player->obj.vel.x = -speed;
		g_map.player->obj.flip_x = true;
	}
	// Move right
	if (keys[SDL_SCANCODE_RIGHT]) {
		g_map.player->obj.vel.x = speed;
		g_map.player->obj.flip_x = false;
	}
	// Jump/Swim
	if (keys[SDL_SCANCODE_S]) {
	}
	// Climb up/Look up
	if (keys[SDL_SCANCODE_UP]) {
		g_map.player->obj.vel.y = -speed;
	}
	// Climb down/Crouch and look down
	if (keys[SDL_SCANCODE_DOWN]) {
		g_map.player->obj.vel.y = speed;
	}

	// Actually move the player
	g_map.player->obj.pos.x += g_map.player->obj.vel.x;
	g_map.player->obj.pos.y += g_map.player->obj.vel.y;

	// Selectively play the player animation
	if (g_map.player->obj.vel.x || g_map.player->obj.vel.y) {
		step_animation(&g_map.player->obj, 2, 3);
	} else {
		g_map.player->obj.sprite_offset = 0;
		g_map.player->obj.ani_delay = 1;
	}

	// Stop the player if in collision with something else
	tile_collision(&g_map.player->obj);

	// Blargysplargh
	obj_collision(&g_map.player->obj, &g_map.objects[0]);

	// Wrap player around if map wraps AFTER collision
	wrap_obj(&g_map.player->obj);
}
