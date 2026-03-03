#ifndef SPRITE_PLAYER_H
#define SPRITE_PLAYER_H

// =============================================
// Sprite: player
// Source: player.png
// Taille: 24x24 pixels (3 octets x 24 lignes)
// Format: 1 octet = 8 pixels (1 bit/pixel)
// Contrainte: 2 couleurs par groupe de 8 pixels
// =============================================

#define SPRITE_PLAYER_WIDTH_BYTES 3
#define SPRITE_PLAYER_HEIGHT 24

// Données de FORME (bitmap: 1=forme, 0=fond)
unsigned char sprite_player_form[72] = {
    0x00, 0x18, 0x00,  // 0  -----------██-----------
    0x00, 0x3C, 0x00,  // 1  ----------████----------
    0x00, 0x7E, 0x00,  // 2  ---------██████---------
    0x00, 0xFF, 0x00,  // 3  --------████████--------
    0x03, 0xFF, 0xC0,  // 4  ------████████████------
    0x07, 0xFF, 0xE0,  // 5  -----██████████████-----
    0x0F, 0xFF, 0xF0,  // 6  ----████████████████----
    0x0F, 0xFF, 0xF0,  // 7  ----████████████████----
    0x0F, 0xC3, 0xF0,  // 8  ----██████----██████----
    0x0F, 0x81, 0xF0,  // 9  ----█████------█████----
    0x0F, 0xC3, 0xF0,  // 10  ----██████----██████----
    0x0F, 0xFF, 0xF0,  // 11  ----████████████████----
    0x1F, 0xFF, 0xF8,  // 12  ---██████████████████---
    0x3F, 0xFF, 0xFC,  // 13  --████████████████████--
    0x7F, 0xFF, 0xFE,  // 14  -██████████████████████-
    0x7F, 0xFF, 0xFE,  // 15  -██████████████████████-
    0xFF, 0xFF, 0xFF,  // 16  ████████████████████████
    0xFF, 0xFF, 0xFF,  // 17  ████████████████████████
    0xE0, 0xFF, 0x07,  // 18  ███-----████████-----███
    0x80, 0xFF, 0x01,  // 19  █-------████████-------█
    0x00, 0xFF, 0x00,  // 20  --------████████--------
    0x00, 0xC3, 0x00,  // 21  --------██----██--------
    0x00, 0xC3, 0x00,  // 22  --------██----██--------
    0x00, 0xC3, 0x00  // 23  --------██----██--------
};

// Données de COULEUR (attributs par groupe de 8 pixels)
// Format: FFFFBBBB (Forme bits 4-7, Fond bits 0-3)
unsigned char sprite_player_color[72] = {
    0x00, 0x40, 0x00,  // 0
    0x00, 0x40, 0x00,  // 1
    0x00, 0x40, 0x00,  // 2
    0x00, 0x40, 0x00,  // 3
    0x40, 0x40, 0x40,  // 4
    0x40, 0x40, 0x40,  // 5
    0x40, 0x40, 0x40,  // 6
    0x40, 0x40, 0x40,  // 7
    0x40, 0x40, 0x40,  // 8
    0x40, 0x40, 0x40,  // 9
    0x40, 0x40, 0x40,  // 10
    0x40, 0x40, 0x40,  // 11
    0x40, 0x40, 0x40,  // 12
    0x40, 0x40, 0x40,  // 13
    0x40, 0x40, 0x40,  // 14
    0x40, 0x40, 0x40,  // 15
    0x10, 0x40, 0x10,  // 16
    0x10, 0x40, 0x10,  // 17
    0x10, 0x40, 0x10,  // 18
    0x10, 0x40, 0x10,  // 19
    0x00, 0x40, 0x00,  // 20
    0x00, 0x40, 0x00,  // 21
    0x00, 0x30, 0x00,  // 22
    0x00, 0x30, 0x00  // 23
};

// Taille totale: 72 octets par tableau
// Blocs multi-couleurs: 37 / 72 (51.4%)

// Combinaisons de couleurs utilisées:
//   Fond=C_BLACK, Forme=C_BLACK : 16 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_RED : 8 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_YELLOW : 2 blocs de 8 pixels
//   Fond=C_BLACK, Forme=C_BLUE : 46 blocs de 8 pixels

// Macro d'initialisation pour MO5_Sprite (voir mo5_sprite.h)
#define SPRITE_PLAYER_INIT \
    { sprite_player_form, sprite_player_color, \
      SPRITE_PLAYER_WIDTH_BYTES, SPRITE_PLAYER_HEIGHT }

// Utilisation:
//   MO5_Sprite sprite_player = SPRITE_PLAYER_INIT;

#endif // SPRITE_PLAYER_H