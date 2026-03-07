#ifndef SPRITE_BULLET_ENEMY_H
#define SPRITE_BULLET_ENEMY_H

// =============================================
// Sprite: bullet_enemy
// Source: bullet_enemy.png
// Taille: 8x8 pixels (1 octets x 8 lignes)
// Format: 1 octet = 8 pixels (1 bit/pixel)
// Contrainte: 2 couleurs par groupe de 8 pixels
// =============================================

#define SPRITE_BULLET_ENEMY_WIDTH_BYTES 1
#define SPRITE_BULLET_ENEMY_HEIGHT 8

// Données de FORME (bitmap: 1=forme, 0=fond)
unsigned char sprite_bullet_enemy_form[8] = {
    0x18,  // 0  ---██---
    0x18,  // 1  ---██---
    0x3C,  // 2  --████--
    0x18,  // 3  ---██---
    0x18,  // 4  ---██---
    0x3C,  // 5  --████--
    0x18,  // 6  ---██---
    0x18  // 7  ---██---
};

// Données de COULEUR (attributs par groupe de 8 pixels)
// Format: FFFFBBBB (Forme bits 4-7, Fond bits 0-3)
unsigned char sprite_bullet_enemy_color[8] = {
    0x10,  // 0
    0x10,  // 1
    0xF0,  // 2
    0xF0,  // 3
    0xF0,  // 4
    0xF0,  // 5
    0x10,  // 6
    0x10  // 7
};

// Taille totale: 8 octets par tableau
// Blocs multi-couleurs: 8 / 8 (100.0%)

// Combinaisons de couleurs utilisées:
//   Fond=C_BLACK, Forme=C_RED : 4 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_ORANGE : 4 blocs de 8 pixels

// Macro d'initialisation pour MO5_Sprite (voir mo5_sprite.h)
#define SPRITE_BULLET_ENEMY_INIT \
    { sprite_bullet_enemy_form, sprite_bullet_enemy_color, \
      SPRITE_BULLET_ENEMY_WIDTH_BYTES, SPRITE_BULLET_ENEMY_HEIGHT }

// Utilisation:
//   MO5_Sprite sprite_bullet_enemy = SPRITE_BULLET_ENEMY_INIT;

#endif // SPRITE_BULLET_ENEMY_H