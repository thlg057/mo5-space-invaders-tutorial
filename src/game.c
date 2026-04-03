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
#include "assets/enemy.h"
#include "assets/bullet_enemy.h"

/* always : x -> octets, y -> pixels */
#define DRAWING_BLOC_HIGH         6   /* pixels */
#define PLAYER_SPEED_X            1   /* octets */
#define BULLET_SPEED_Y            4   /* pixels */
#define MAX_BULLETS_PLAYER        3
#define PLAYER_MAX_LIFE           3
#define PLAYER_MAX_SCORE          50
#define ENEMY_COUNT               4   /* doit rester une puissance de 2 pour le masque rand */
#define ENEMY_COUNT_MASK          3   /* ENEMY_COUNT - 1 : remplace % ENEMY_COUNT */
#define ENEMY_DIRECTION_RIGHT     1
#define ENEMY_DIRECTION_LEFT      2
#define ENEMY_SPEED_X             1   /* octets */
#define ENEMY_SPEED_Y             4   /* pixels */
#define ENEMY_FRAME_SPEED         8
#define MAX_BULLETS_ENEMIES       4
#define BULLET_FRAME_SPEED        10
#define PLAYER_INVINCIBLE_FRAMES  60
/* Bit 3 de player_invincible : bascule toutes les 8 frames (clignotement) */
#define PLAYER_BLINK_BIT          0x08

/*
 * Codes de retour de game_check_collisions()
 */
#define GAME_RESULT_CONTINUE   0
#define GAME_RESULT_GAME_OVER  1

/* -------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------- */

typedef struct {
    MO5_Actor      actor;
    unsigned char  active; /* 0 = slot disponible */
} ActiveActor;

/* -------------------------------------------------------------------------
 * Sprites (données graphiques statiques)
 * ---------------------------------------------------------------------- */

static MO5_Sprite  player_sprite        = SPRITE_PLAYER_INIT;
static MO5_Sprite  bullet_player_sprite = SPRITE_BULLET_PLAYER_INIT;
static MO5_Sprite  enemy_sprite         = SPRITE_ENEMY_INIT;
static MO5_Sprite  bullet_enemy_sprite  = SPRITE_BULLET_ENEMY_INIT;

/* -------------------------------------------------------------------------
 * État global du jeu
 * ---------------------------------------------------------------------- */

static MO5_Actor   player;
static ActiveActor bullets_player[MAX_BULLETS_PLAYER];
static ActiveActor enemies[ENEMY_COUNT];
static ActiveActor bullets_enemies[MAX_BULLETS_ENEMIES];

static unsigned char enemy_direction  = ENEMY_DIRECTION_RIGHT;
static unsigned char rand_seed        = 1;
static unsigned char player_invincible = 0;

/* Score et vies : dirty flags pour éviter tout redraw inutile */
static unsigned char g_score;
static unsigned char g_live;
static unsigned char g_score_dirty;
static unsigned char g_live_dirty;

/* Variables de game_loop() promues en statiques globales (règle #14 CMOC :
 * évite le débordement de stack à l'entrée de la fonction) */
static unsigned char gl_new_x;
static unsigned char gl_result;
static unsigned char gl_enemies_tick;
static unsigned char gl_bullets_tick;
static unsigned char gl_i;
static char          gl_key;

/* =========================================================================
 * Utilitaires internes
 * ====================================================================== */

static unsigned char collide(unsigned char ax, unsigned char ay,
                              unsigned char aw, unsigned char ah,
                              unsigned char bx, unsigned char by,
                              unsigned char bw, unsigned char bh)
{
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

/*
 * Générateur pseudo-aléatoire : LFSR 8-bit, polynôme x^8+x^6+x^5+x^4+1.
 * Léger et sans division — parfait pour le 6809.
 */
static unsigned char pseudo_rand(void)
{
    unsigned char feedback;
    feedback  = ((rand_seed >> 7) & 1) ^
                ((rand_seed >> 5) & 1) ^
                ((rand_seed >> 4) & 1) ^
                ((rand_seed >> 3) & 1);
    rand_seed = (rand_seed << 1) | feedback;
    return rand_seed;
}

/* =========================================================================
 * Initialisation
 * ====================================================================== */

static void game_init_player(void)
{
    unsigned char i;

    player.sprite  = &player_sprite;
    player.pos.x   = (SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES) >> 1;
    player.pos.y   = SCREEN_HEIGHT - SPRITE_PLAYER_HEIGHT;
    player.old_pos = player.pos;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        bullets_player[i].active       = 0;
        bullets_player[i].actor.sprite = &bullet_player_sprite;
    }
}

static void game_init_enemies(void)
{
    /* FIX : la multiplication i*(w+spacing) est remplacée par une
     * accumulation — zéro opération coûteuse sur 6809. */
    unsigned char i;
    unsigned char spacing;
    unsigned char cur_x;

    spacing = (SCREEN_WIDTH_BYTES - (ENEMY_COUNT * SPRITE_ENEMY_WIDTH_BYTES))
              / (ENEMY_COUNT + 1);

    cur_x = spacing;
    for (i = 0; i < ENEMY_COUNT; i++) {
        enemies[i].active            = 1;
        enemies[i].actor.sprite      = &enemy_sprite;
        enemies[i].actor.pos.x       = cur_x;
        enemies[i].actor.pos.y       = DRAWING_BLOC_HIGH;
        enemies[i].actor.old_pos     = enemies[i].actor.pos;
        cur_x += SPRITE_ENEMY_WIDTH_BYTES + spacing;
    }

    for (i = 0; i < MAX_BULLETS_ENEMIES; i++) {
        bullets_enemies[i].active       = 0;
        bullets_enemies[i].actor.sprite = &bullet_enemy_sprite;
    }
}

/* =========================================================================
 * Tirs joueur
 * ====================================================================== */

static void game_fire_player_bullet(void)
{
    unsigned char i;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (bullets_player[i].active) continue;

        bullets_player[i].actor.pos.x   = player.pos.x + 1;
        bullets_player[i].actor.pos.y   = player.pos.y - SPRITE_BULLET_PLAYER_HEIGHT;
        bullets_player[i].actor.old_pos = bullets_player[i].actor.pos;
        bullets_player[i].active        = 1;

        mo5_actor_draw_bg(&bullets_player[i].actor);
        return;
    }
}

/* FIX : renommage palyer -> player */
static void game_update_player_bullets(void)
{
    const unsigned char max_y = DRAWING_BLOC_HIGH + BULLET_SPEED_Y;
    unsigned char i;

    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (!bullets_player[i].active) continue;

        if (bullets_player[i].actor.pos.y >= max_y) {
            mo5_actor_move_bg(&bullets_player[i].actor,
                              bullets_player[i].actor.pos.x,
                              bullets_player[i].actor.pos.y - BULLET_SPEED_Y);
            continue;
        }

        mo5_actor_clear_bg(&bullets_player[i].actor);
        bullets_player[i].active = 0;
    }
}

/* =========================================================================
 * Ennemis
 * ====================================================================== */

static void game_update_enemies(void)
{
    unsigned char i;
    unsigned char need_reverse;
    unsigned char new_x;
    unsigned char new_y;

    need_reverse = 0;

    /* Détection rebord */
    for (i = 0; i < ENEMY_COUNT; i++) {
        if (!enemies[i].active) continue;

        if (enemy_direction == ENEMY_DIRECTION_RIGHT) {
            if (enemies[i].actor.pos.x + SPRITE_ENEMY_WIDTH_BYTES >= SCREEN_WIDTH_BYTES) {
                need_reverse = 1;
                break;
            }
        } else {
            if (enemies[i].actor.pos.x == 0) {
                need_reverse = 1;
                break;
            }
        }
    }

    if (need_reverse)
        enemy_direction = (enemy_direction == ENEMY_DIRECTION_RIGHT)
                        ? ENEMY_DIRECTION_LEFT
                        : ENEMY_DIRECTION_RIGHT;

    /* Déplacement */
    for (i = 0; i < ENEMY_COUNT; i++) {
        if (!enemies[i].active) continue;

        new_x = enemies[i].actor.pos.x;
        new_y = enemies[i].actor.pos.y;

        if (enemy_direction == ENEMY_DIRECTION_RIGHT)
            new_x += ENEMY_SPEED_X;
        else
            new_x -= ENEMY_SPEED_X;

        if (need_reverse)
            new_y += ENEMY_SPEED_Y;

        if (new_y + SPRITE_ENEMY_HEIGHT >= SCREEN_HEIGHT) {
            mo5_actor_clear_bg(&enemies[i].actor);
            enemies[i].active = 0;
            continue;
        }

        mo5_actor_move_bg(&enemies[i].actor, new_x, new_y);
    }
}

/* =========================================================================
 * Tirs ennemis
 * ====================================================================== */

static void game_fire_enemy_bullet(unsigned char enemy_idx)
{
    unsigned char i;

    for (i = 0; i < MAX_BULLETS_ENEMIES; i++) {
        if (bullets_enemies[i].active) continue;

        bullets_enemies[i].actor.pos.x   = enemies[enemy_idx].actor.pos.x;
        bullets_enemies[i].actor.pos.y   = enemies[enemy_idx].actor.pos.y + SPRITE_ENEMY_HEIGHT;
        bullets_enemies[i].actor.old_pos = bullets_enemies[i].actor.pos;
        bullets_enemies[i].active        = 1;

        mo5_actor_draw_bg(&bullets_enemies[i].actor);
        return;
    }
}

static void game_try_enemy_fire(void)
{
    unsigned char shooter;

    /* FIX : % ENEMY_COUNT (division logicielle) remplacé par un masque binaire.
     * Valide car ENEMY_COUNT == 4 (puissance de 2). */
    shooter = pseudo_rand() & ENEMY_COUNT_MASK;

    if (enemies[shooter].active)
        game_fire_enemy_bullet(shooter);
}

static void game_update_enemies_bullets(void)
{
    unsigned char i;
    unsigned char max;

    max = SCREEN_HEIGHT - BULLET_SPEED_Y;

    for (i = 0; i < MAX_BULLETS_ENEMIES; i++) {
        if (!bullets_enemies[i].active) continue;

        if (bullets_enemies[i].actor.pos.y < max) {
            mo5_actor_move_bg(&bullets_enemies[i].actor,
                              bullets_enemies[i].actor.pos.x,
                              bullets_enemies[i].actor.pos.y + BULLET_SPEED_Y);
            continue;
        }

        mo5_actor_clear_bg(&bullets_enemies[i].actor);
        bullets_enemies[i].active = 0;
    }
}

/* =========================================================================
 * Affichage HUD
 * ====================================================================== */

/*
 * Conversion entier -> 2 chiffres décimaux sans division ni modulo.
 * Valide pour des valeurs <= 99.
 * Les deux caractères sont écrits dans buf[0] (dizaine) et buf[1] (unité).
 */
static void uint8_to_dec2(char *buf, unsigned char val)
{
    unsigned char tens;
    tens = 0;
    while (val >= 10) { val -= 10; tens++; }
    buf[0] = '0' + tens;
    buf[1] = '0' + val;
    buf[2] = '\0';
}

static void game_display_score(void)
{
    char buf[3];
    uint8_to_dec2(buf, g_score);
    mo5_fill_rect(0, 0, 9, 6, GAME_BACKGROUND_COLOR);
    mo5_font6_puts(0, 0, "score:", C_BLUE);
    mo5_font6_puts(6, 0, buf, C_BLUE);
}

static void game_display_live(void)
{
    char buf[2];
    buf[0] = '0' + g_live;
    buf[1] = '\0';
    mo5_fill_rect(35, 0, 5, 6, GAME_BACKGROUND_COLOR);
    mo5_font6_puts(35, 0, "VIE:", C_BLUE);
    mo5_font6_puts(39, 0, buf, C_BLUE);
}

/* =========================================================================
 * Redraw complet de la scène (après pause)
 * ====================================================================== */

static void game_redraw_enemies_and_bullets(void)
{
    unsigned char i;

    for (i = 0; i < ENEMY_COUNT; i++) {
        if (enemies[i].active)
            mo5_actor_draw_bg(&enemies[i].actor);
    }
    for (i = 0; i < MAX_BULLETS_ENEMIES; i++) {
        if (bullets_enemies[i].active)
            mo5_actor_draw_bg(&bullets_enemies[i].actor);
    }
    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (bullets_player[i].active)
            mo5_actor_draw_bg(&bullets_player[i].actor);
    }
}

/* =========================================================================
 * Gestion des écrans (pause, game over, victoire)
 * ====================================================================== */

static unsigned char game_quit_game(void)
{
    char key;

    mo5_fill_rect(7, 90, 25, 26, GAME_MESSAGE_BACKGROUND_COLOR);
    mo5_font6_puts(8, 100, "Quitter la partie ? Y/N", GAME_MESSAGE_COLOR);
    key = mo5_wait_for_key();
    if (key == 'Y')
        return 1;

    mo5_fill_rect(7, 90, 25, 26, GAME_BACKGROUND_COLOR);
    game_redraw_enemies_and_bullets();
    return 0;
}

void game_ready_to_start(void)
{
    mo5_fill_rect(7, 90, 26, 26, GAME_MESSAGE_BACKGROUND_COLOR);
    mo5_font6_puts(8, 100, "Press space bar to start", GAME_MESSAGE_COLOR);
    mo5_wait_key(' ');
    mo5_fill_rect(7, 90, 26, 26, GAME_BACKGROUND_COLOR);
}

static void game_show_game_over(void)
{
    mo5_fill_rect(7, 90, 25, 26, GAME_MESSAGE_BACKGROUND_COLOR);
    mo5_font6_puts(15, 95,  "GAME OVER",              GAME_MESSAGE_LOSE_COLOR);
    mo5_font6_puts(8,  107, "Press space to continue", GAME_MESSAGE_COLOR);
    mo5_wait_key(' ');
}

static void game_show_victory(void)
{
    mo5_fill_rect(7, 90, 25, 26, GAME_MESSAGE_BACKGROUND_COLOR);
    mo5_font6_puts(15, 95,  "VICTOIRE !",              GAME_MESSAGE_WIN_COLOR);
    mo5_font6_puts(8,  107, "Press space to continue", GAME_MESSAGE_COLOR);
    mo5_wait_key(' ');
}

/* =========================================================================
 * Collisions
 * Retourne GAME_RESULT_GAME_OVER ou GAME_RESULT_CONTINUE.
 * N'affiche rien : les dirty flags sont traités dans game_loop().
 * ====================================================================== */

static unsigned char game_check_collisions(void)
{
    unsigned char i;
    unsigned char j;

    /* --- Tirs joueur vs ennemis --- */
    for (i = 0; i < MAX_BULLETS_PLAYER; i++) {
        if (!bullets_player[i].active) continue;
        for (j = 0; j < ENEMY_COUNT; j++) {
            if (!enemies[j].active) continue;
            if (collide(bullets_player[i].actor.pos.x,
                        bullets_player[i].actor.pos.y,
                        SPRITE_BULLET_PLAYER_WIDTH_BYTES,
                        SPRITE_BULLET_PLAYER_HEIGHT,
                        enemies[j].actor.pos.x,
                        enemies[j].actor.pos.y,
                        SPRITE_ENEMY_WIDTH_BYTES,
                        SPRITE_ENEMY_HEIGHT))
            {
                mo5_actor_clear_bg(&bullets_player[i].actor);
                bullets_player[i].active = 0;
                mo5_actor_clear_bg(&enemies[j].actor);
                enemies[j].active = 0;
                g_score++;
                g_score_dirty = 1;
                break;
            }
        }
    }

    /* --- Tirs ennemis vs joueur --- */
    if (player_invincible == 0) {
        for (i = 0; i < MAX_BULLETS_ENEMIES; i++) {
            if (!bullets_enemies[i].active) continue;
            if (collide(bullets_enemies[i].actor.pos.x,
                        bullets_enemies[i].actor.pos.y,
                        SPRITE_BULLET_ENEMY_WIDTH_BYTES,
                        SPRITE_BULLET_ENEMY_HEIGHT,
                        player.pos.x,
                        player.pos.y,
                        SPRITE_PLAYER_WIDTH_BYTES,
                        SPRITE_PLAYER_HEIGHT))
            {
                mo5_actor_clear_bg(&bullets_enemies[i].actor);
                bullets_enemies[i].active = 0;
                g_live--;
                g_live_dirty      = 1;
                player_invincible = PLAYER_INVINCIBLE_FRAMES;
                if (g_live == 0) return GAME_RESULT_GAME_OVER;
                break;
            }
        }
    }

    /* --- Ennemis vs joueur (contact direct) --- */
    if (player_invincible == 0) {
        for (i = 0; i < ENEMY_COUNT; i++) {
            if (!enemies[i].active) continue;
            if (collide(enemies[i].actor.pos.x,
                        enemies[i].actor.pos.y,
                        SPRITE_ENEMY_WIDTH_BYTES,
                        SPRITE_ENEMY_HEIGHT,
                        player.pos.x,
                        player.pos.y,
                        SPRITE_PLAYER_WIDTH_BYTES,
                        SPRITE_PLAYER_HEIGHT))
            {
                g_live--;
                g_live_dirty      = 1;
                player_invincible = PLAYER_INVINCIBLE_FRAMES;
                if (g_live == 0) return GAME_RESULT_GAME_OVER;
                break;
            }
        }
    }

    return GAME_RESULT_CONTINUE;
}

/* =========================================================================
 * Boucle principale
 * Les variables locales sont promues en statiques globales (gl_) pour ne
 * pas dépasser le budget stack de CMOC à l'entrée de la fonction.
 * ====================================================================== */

void game_loop(void)
{
    const unsigned char max_x = SCREEN_WIDTH_BYTES - SPRITE_PLAYER_WIDTH_BYTES;

    game_init_player();
    game_init_enemies();

    gl_new_x        = player.pos.x;
    g_score         = 0;
    g_live          = PLAYER_MAX_LIFE;
    g_score_dirty   = 0;
    g_live_dirty    = 0;
    gl_enemies_tick = 0;
    gl_bullets_tick = 0;

    game_display_score();
    game_display_live();
    mo5_actor_draw_bg(&player);
    for (gl_i = 0; gl_i < ENEMY_COUNT; gl_i++) {
        if (enemies[gl_i].active)
            mo5_actor_draw_bg(&enemies[gl_i].actor);
    }

    game_ready_to_start();

    while (1) {
        /* Victoire : score atteint */
        if (g_score == PLAYER_MAX_SCORE) {
            game_show_victory();
            return;
        }

        gl_enemies_tick++;
        gl_bullets_tick++;

        /* Lecture input */
        gl_key = mo5_getchar();
        switch (gl_key) {
            case 'Q':
                if (gl_new_x >= PLAYER_SPEED_X)
                    gl_new_x -= PLAYER_SPEED_X;
                else
                    gl_new_x = 0;
                break;
            case 'D':
                if (gl_new_x + PLAYER_SPEED_X <= max_x)
                    gl_new_x += PLAYER_SPEED_X;
                else
                    gl_new_x = max_x;
                break;
            case ' ':
                game_fire_player_bullet();
                break;
            case 'P':
                if (game_quit_game() == 1)
                    return;
                break;
        }

        mo5_wait_vbl();

        /* Mise à jour ennemis (throttlée) */
        if (gl_enemies_tick == ENEMY_FRAME_SPEED) {
            gl_enemies_tick = 0;
            game_update_enemies();
        }

        /* Mise à jour tirs ennemis */
        game_update_enemies_bullets();

        if (gl_bullets_tick >= BULLET_FRAME_SPEED) {
            gl_bullets_tick = 0;
            game_try_enemy_fire();
        }

        /* Mise à jour tirs joueur et position joueur */
        game_update_player_bullets();
        mo5_actor_move_bg(&player, gl_new_x, player.pos.y);

        /* Collisions — affichage score/vie toujours depuis ici via dirty flags */
        gl_result = game_check_collisions();
        if (g_score_dirty) { game_display_score(); g_score_dirty = 0; }
        if (g_live_dirty)  { game_display_live();  g_live_dirty  = 0; }
        if (gl_result == GAME_RESULT_GAME_OVER) { game_show_game_over(); return; }

        /* Clignotement invincibilité.
         * FIX : % 12 (division logicielle) remplacé par un test de bit.
         * PLAYER_BLINK_BIT (0x08) bascule toutes les 8 frames. */
        if (player_invincible > 0) {
            player_invincible--;
            if (player_invincible == 0) {
                mo5_actor_draw_bg(&player); /* fin invincibilité : redessiner */
            } else if (player_invincible & PLAYER_BLINK_BIT) {
                mo5_actor_clear_bg(&player);
            } else {
                mo5_actor_draw_bg(&player);
            }
        }

        /* Résurrection des ennemis détruits.
         * FIX : ajout du draw et remise à zéro de old_pos pour éviter
         * un move_bg sur un acteur qui n'aurait jamais été dessiné. */
        for (gl_i = 0; gl_i < ENEMY_COUNT; gl_i++) {
            if (!enemies[gl_i].active) {
                enemies[gl_i].actor.pos.y   = DRAWING_BLOC_HIGH;
                //enemies[gl_i].actor.old_pos = enemies[gl_i].actor.pos;
                enemies[gl_i].active        = 1;
                //mo5_actor_draw_bg(&enemies[gl_i].actor);
            }
        }
    }
}