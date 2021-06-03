// Super Grayland, Copyright 2020 Vincent Robinson under the MIT license. See 'LICENSE.txt' for more information.

#pragma once

#include "common.h"
#include "game.h"
#include "obj.h"
#include "screen.h"

// TODO: Remove obj reference in player struct
struct player
{
	struct obj obj;
};

#define DEF_GRAYFORD {		\
	.sprite_pos = {0, 0},	\
	.rect_size = {6, 8},	\
	.rect_offset = {1, 0}	\
}

#define DEF_SUPER_GRAYFORD {\
	.sprite_pos = {4, 0},	\
	.rect_size = {6, 11},	\
	.rect_offset = {1, 5},	\
	.extra_sprite.y = 1		\
}

// Update the player
void update_player(const u8 *keys);
