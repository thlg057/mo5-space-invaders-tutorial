/*
 * MIT License
 *
 * Copyright (c) 2025 Thierry Le Got
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <mo5_defs.h>
#include <mo5_sprite_bg.h>
#include "game.h"
#include "assets/player.h"

#define PLAYER_SPEED_X      1 //octet

static MO5_Sprite player_sprite        = SPRITE_PLAYER_INIT;
static MO5_Actor  player;

static void game_init_player(void)
{
    unsigned char i;
    player.sprite  = &player_sprite;
    player.pos.x   = (SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES) / 2;
    player.pos.y   = SCREEN_HEIGHT - SPRITE_PLAYER_HEIGHT;
    player.old_pos = player.pos;
}

void game_loop(void) {
    const unsigned char max_x = SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES;;
    char key;
    unsigned char new_x;

    game_init_player();
    mo5_actor_draw_bg(&player);
    new_x = player.pos.x;

    while(1) {
        key = mo5_getchar();
        switch (key) {
            case 'Q':
                new_x = (new_x >= PLAYER_SPEED_X) ? new_x - PLAYER_SPEED_X : 0;
                break;
            case 'D':
                new_x = (new_x + PLAYER_SPEED_X <= max_x) ? new_x + PLAYER_SPEED_X : max_x;
                break;
        }

        mo5_wait_vbl();
        mo5_actor_move_bg(&player, new_x, player.pos.y);
    }
}