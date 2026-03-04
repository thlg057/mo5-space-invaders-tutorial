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
#include <mo5_font6.h>
#include "game.h"
#include "assets/player.h"
#include "assets/bullet_player.h"

#define DRAWING_BLOC_HIGH   6
#define MAX_BULLETS_PLAYER  3
#define PLAYER_SPEED_X      1
#define BULLET_SPEED_Y      4
#define PLAYER_MAX_LIFE     3

typedef struct {
    MO5_Actor     actor;
    unsigned char active; //0 -> slot dispo
} BulletActor;

static MO5_Sprite player_sprite        = SPRITE_PLAYER_INIT;
static MO5_Sprite bullet_player_sprite = SPRITE_BULLET_PLAYER_INIT;

static MO5_Actor   player_actor;
static BulletActor bullets_player[MAX_BULLETS_PLAYER];

static void game_init_player(void)
{
    unsigned char i;

    player_actor.sprite  = &player_sprite;
    player_actor.pos.x   = (SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES) / 2;
    player_actor.pos.y   = SCREEN_HEIGHT - SPRITE_PLAYER_HEIGHT;
    player_actor.old_pos = player_actor.pos;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        bullets_player[i].active       = 0;
        bullets_player[i].actor.sprite = &bullet_player_sprite;
    }
}

static void game_fire_bullet(void)
{
    unsigned char i;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (bullets_player[i].active) continue;

        bullets_player[i].actor.pos.x   = player_actor.pos.x + 1;
        bullets_player[i].actor.pos.y   = player_actor.pos.y - SPRITE_BULLET_PLAYER_HEIGHT;
        bullets_player[i].actor.old_pos = bullets_player[i].actor.pos;
        bullets_player[i].active        = 1;

        mo5_actor_draw_bg(&bullets_player[i].actor);
        return;
    }
}

static void game_update_bullets(void)
{
    unsigned char i;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (!bullets_player[i].active) continue;

        if (bullets_player[i].actor.pos.y >= DRAWING_BLOC_HIGH + BULLET_SPEED_Y) {
            mo5_actor_move_bg(&bullets_player[i].actor,
                              bullets_player[i].actor.pos.x,
                              bullets_player[i].actor.pos.y - BULLET_SPEED_Y);
            continue;
        
        }

        mo5_actor_clear_bg(&bullets_player[i].actor);
        bullets_player[i].active = 0;
    }
}

static void display_score(unsigned char score) {
    // sprintf fait planter le programme.
    // char buf[12];
    // sprintf(buf, "SCORE: %03u", score);
    // mo5_font6_puts(0, 0, buf, C_BLUE);
    char buf[4];
    buf[0] = '0' + (score / 100);
    buf[1] = '0' + (score % 100 / 10);
    buf[2] = '0' + (score % 10);
    buf[3] = '\0';

    mo5_font6_puts(0, 0, "SCORE:", C_BLUE);
    mo5_font6_puts(6, 0, buf, C_BLUE);
}

static void display_live(unsigned char live) {
    // sprintf fait planter le programme.
    // char buf[7];
    // sprintf(buf, "VIE: %u", live);
    // mo5_font6_puts(35, 0, buf, C_BLUE);
    char buf[2];
    buf[0] = '0' + live;
    buf[1] = '\0';
    mo5_font6_puts(35, 0, "VIE:", C_BLUE);
    mo5_font6_puts(39, 0, buf, C_BLUE);
}

void game_loop(void)
{
    const unsigned char max_x = SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES;
    unsigned char score = 0;
    unsigned char live = PLAYER_MAX_LIFE;
    game_init_player();
    mo5_actor_draw_bg(&player_actor);

    display_score(score);
    display_live(live);

    unsigned char input;
    unsigned char new_x = player_actor.pos.x;
    while (1) {
        mo5_wait_vbl();
        char key = mo5_getchar();
        switch (key) {
            case 'Q':
                new_x = (new_x >= PLAYER_SPEED_X) ? new_x - PLAYER_SPEED_X : 0;
                break;
            case 'D':
                new_x = (new_x + PLAYER_SPEED_X <= max_x) ? new_x + PLAYER_SPEED_X : max_x;
                break;
            case ' ':
                game_fire_bullet();
                break;
        }

        game_update_bullets();
        mo5_actor_move_bg(&player_actor, new_x, player_actor.pos.y);
    }
}