# Développer un Space Invaders pour Thomson MO5 en C avec CMOC

Ce tutorial montre comment développer un shoot'em up vertical sur Thomson MO5, etape par etape, en C compile avec CMOC. Chaque etape correspond a un tag Git.

## Prerequis

- CMOC installe et fonctionnel
- Emulateur MO5 (par exemple YAME)
- Repo de base : `https://github.com/thlg057/mo5_template`
- Notions de base en C (pointeurs, tableaux statiques)

## La machine

| Caracteristique | Valeur |
|---|---|
| CPU | Motorola 6809 @ ~1 MHz |
| RAM utile | 48 Ko |
| Resolution | 320x200 pixels |
| Couleurs | 16 couleurs |
| Compilateur | CMOC (C pour 6809) |

Deux regles fondamentales s'appliquent a tout le code.

**Pas de malloc.** Toute la memoire est allouee statiquement a la compilation. Les tableaux d'ennemis, de balles, de sprites -- tout est dimensionne a l'avance avec des `#define`.

**Pas de float, pas de division si possible.** Le 6809 ne dispose pas d'unite virgule flottante. On prefere les decalages (`>> 1` au lieu de `/ 2`) et l'arithmetique sur `unsigned char` (8 bits natifs) plutot que `int` (16 bits).

## Contrainte graphique : 2 couleurs par bloc de 8 pixels

La VRAM du MO5 est organisee en deux banques selectionnees via le registre `PRC` :

- **Banque couleur** : chaque octet encode la couleur de fond (bits 0-3) et la couleur de forme (bits 4-7) pour un groupe de 8 pixels horizontaux.
- **Banque forme** : chaque bit correspond a un pixel -- `1` affiche la couleur de forme, `0` affiche la couleur de fond.

Un groupe de 8 pixels ne peut afficher que **2 couleurs**. Les sprites respectent cette contrainte en variant les couleurs ligne par ligne. Le script `png2mo5.py` verifie et encode cette regle lors de la generation des `.h`.

## SDK maison

| Fichier | Role |
|---|---|
| `mo5_defs.h` | Registres hardware, palette, dimensions ecran |
| `mo5_video.h` | Init video, clear ecran, fill rectangle |
| `mo5_sprite_bg.h` | Sprites transparents (fond conserve) |
| `mo5_sprite_form.h` | Sprites forme seule |
| `mo5_sprite.h` | Sprites opaques |
| `mo5_actor_dr.h` | Dirty rectangle : save/restore zone VRAM |
| `mo5_font6.h` | Affichage de texte - 6 pixels de haut |
| `mo5_font8.h` | Affichage de texte - 8 pixels de haut |

Pipeline de conversion sprite :

```bash
make convert IMG=./assets/player.png
# genere include/assets/player.h
```

Pour ce projet, on utilisera les fonctions de mo5_sprite_bg.h, qui permettent de gérer la transparence (le fond est celui du décor)

### Sprites disponibles

| Fichier | Taille | Description |
|---|---|---|
| `player.png` | 24x24 | Vaisseau joueur, bleu/rouge/jaune |
| `enemy.png` | 16x16 | Alien insecte, vert/jaune |
| `bullet_player.png` | 8x8 | Tir joueur, cyan/blanc |
| `bullet_enemy.png` | 8x8 | Tir ennemi, rouge/orange |

### Structures de donnees

`unsigned char` partout ou la valeur tient sur 8 bits.

```c
typedef struct {
    MO5_Actor     actor;
    unsigned char active;
} ActiveActor;

typedef struct {
    MO5_Actor     actor;
    unsigned char active;
    unsigned char hp;
} EnemyActor;
```

Les coordonnees `x` sont en **octets** (1 unite = 8 pixels), les coordonnees `y` sont en **pixels**. Cette distinction est imposee par la VRAM.

## Architecture du jeu

Le jeu repose sur une boucle principale simple :

1.  Lecture des entrées clavier
2.  Mise à jour joueur
3.  Mise à jour balles
4.  Mise à jour ennemis
5.  Détection collisions
6.  Attente VBL
7.  Dessin

Principe fondamental MO5 :

-   Pas d'allocation dynamique
-   Structures statiques dimensionnées à la compilation
-   Utilisation prioritaire de `unsigned char`
-   Synchronisation à 50 Hz (PAL)

## Organisation des sources

Structure du projet :

    mo5-spaceinvaders/
    ├── Makefile
    ├── src/
    │   ├── main.c
    │   └── game.c
    ├── include/
    │   ├── game.h
    │   └── assets/
    │       ├── player.h
    │       ├── enemy.h
    │       ├── bullet_player.h
    │       └── bullet_enemy.h
    ├── assets/
    │   ├── player.png
    │   ├── enemy.png
    │   ├── bullet_player.png
    │   └── bullet_enemy.png
    ├── output/
    └── tools/

### Rôles

-   **main.c** : point d'entrée minimal (initialisation + appel `game_loop()`)
-   **game.c** : toute la logique du jeu (moteur, update, collisions, HUD)
-   **game.h** : interface publique minimale (`void game_loop(void);`)
-   **include/assets/** : sprites générés automatiquement depuis les PNG

## Utilisation avec GitHub Codespaces

### Créer un Codespace

Depuis le dépôt GitHub : - Cliquer sur **use this tempalte** et sélectionner **Open in a codespace**

### Installer l'environnement

Dans le terminal :

``` bash
make setup-codespace
```

Cette commande installe automatiquement :

-   CMOC (compilateur C 6809)
-   lwtools (assembleur)
-   Python 3 + Pillow
-   Dépendances nécessaires au SDK

### Installation du SDK

Une seule fois où après une mise à jour du SDK.

``` bash
make install
```

Cela : - Télécharge BootFloppyDisk - Compile le sdk_mo5 - Installe les outils dans `tools/`

### Build du projet et génération des images *

``` bash
make
```

## Tags Git

| Tag | Contenu |
|---|---|
| `step-01-player` | Joueur, deplacement, tirs joueur |
| `step-02-score` | HUD score et vies |
| `step-03-enemies` | Ennemis, formation, tirs ennemis |
| `step-04-collisions` | Collisions, points de vie, game over |

## Etape 1 -- `step-01-player` -- Joueur, deplacement et tirs

Afficher le vaisseau joueur en bas de l'ecran, le deplacer avec `Q`/`D`, et tirer des balles vers le haut avec `Espace`.

## Etape 2 -- `step-02-score` -- Affichage du score

Afficher le score et les vies en haut de l'ecran, mis a jour a chaque evenement (ennemi detruit, vie perdue).

## Etape 3 -- `step-03-enemies` -- Ennemis et leurs tirs

Afficher une vague d'ennemis en formation, les faire osciller horizontalement, descendre au rebord, et tirer periodiquement.

## Etape 4 -- `step-04-collisions` -- Collisions, points de vie et fin de partie

Detecter les impacts, gerer les points de vie, l'invincibilite temporaire du joueur, le game over et la victoire.


## Recapitulatif des optimisations MO5

| Regle | Pourquoi |
|---|---|
| `unsigned char` plutot que `int` | Operations 8 bits natives sur 6809 |
| `while (n--)` plutot que `for (i=0; i<n; i++)` | Flag Z automatique apres decrementation |
| `*p++` plutot que `tab[i]` | `LEAX 1,X` au lieu d'un calcul d'index |
| `static` sur les fonctions internes | Inlining possible, symboles non exportes |
| Compteur de frames pour les vitesses | Independant du nombre de sprites actifs |
| Pas de `row_offsets[]` | Economise 400 octets RAM |
| Flag `score_dirty` pour le HUD | Redessinage uniquement en cas de changement |
| `#define` internes dans `game.c` | Pas dans `game.h` si personne d'autre n'en a besoin |