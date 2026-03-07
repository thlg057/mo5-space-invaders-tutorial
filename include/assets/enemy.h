#ifndef SPRITE_ENEMY_H
#define SPRITE_ENEMY_H

// =============================================
// Sprite: enemy
// Source: enemy.png
// Taille: 16x16 pixels (2 octets x 16 lignes)
// Format: 1 octet = 8 pixels (1 bit/pixel)
// Contrainte: 2 couleurs par groupe de 8 pixels
// =============================================

#define SPRITE_ENEMY_WIDTH_BYTES 2
#define SPRITE_ENEMY_HEIGHT 16

// Données de FORME (bitmap: 1=forme, 0=fond)
unsigned char sprite_enemy_form[32] = {
    0x20, 0x20,  // 0  --█-------█-----
    0x30, 0x60,  // 1  --██-----██-----
    0x1D, 0xC0,  // 2  ---███-███------
    0x1F, 0xC0,  // 3  ---███████------
    0x3F, 0xF8,  // 4  --███████████---
    0x1F, 0xF0,  // 5  ---█████████----
    0x1F, 0xF0,  // 6  ---█████████----
    0xFF, 0xFC,  // 7  ██████████████--
    0xFF, 0xFC,  // 8  ██████████████--
    0x1F, 0xF0,  // 9  ---█████████----
    0x17, 0xE0,  // 10  ---█-██████-----
    0x65, 0xA0,  // 11  -██--█-██-█-----
    0x44, 0x20,  // 12  -█---█----█-----
    0x44, 0x20,  // 13  -█---█----█-----
    0x02, 0x40,  // 14  ------█--█------
    0x02, 0x40  // 15  ------█--█------
};

// Données de COULEUR (attributs par groupe de 8 pixels)
// Format: FFFFBBBB (Forme bits 4-7, Fond bits 0-3)
unsigned char sprite_enemy_color[32] = {
    0x20, 0x20,  // 0
    0x20, 0x20,  // 1
    0x20, 0x20,  // 2
    0x20, 0x20,  // 3
    0x20, 0x20,  // 4
    0x70, 0x70,  // 5
    0x70, 0x70,  // 6
    0x20, 0x20,  // 7
    0x20, 0x20,  // 8
    0x30, 0x30,  // 9
    0x30, 0x30,  // 10
    0x20, 0x30,  // 11
    0x20, 0x30,  // 12
    0x20, 0x30,  // 13
    0x30, 0x30,  // 14
    0x30, 0x30  // 15
};

// Taille totale: 32 octets par tableau
// Blocs multi-couleurs: 32 / 32 (100.0%)

// Combinaisons de couleurs utilisées:
//   Fond=C_BLACK, Forme=C_GREEN : 17 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_YELLOW : 11 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_WHITE : 4 blocs de 8 pixels

// Macro d'initialisation pour MO5_Sprite (voir mo5_sprite.h)
#define SPRITE_ENEMY_INIT \
    { sprite_enemy_form, sprite_enemy_color, \
      SPRITE_ENEMY_WIDTH_BYTES, SPRITE_ENEMY_HEIGHT }

// Utilisation:
//   MO5_Sprite sprite_enemy = SPRITE_ENEMY_INIT;

#endif // SPRITE_ENEMY_H