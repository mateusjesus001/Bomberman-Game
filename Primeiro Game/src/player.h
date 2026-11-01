#ifndef PLAYER_H
#define PLAYER_H

#include "bomber.h"
#include "map.h"
#include "bomb.h"

/* Player power-up state */
typedef struct {
    int maxBombs;        /* bomb capacity            */
    int fireRadius;      /* explosion radius in tiles */
    int speedLevel;      /* 0..4 walk speed           */
    bool wallPass;
    bool bombPass;
    bool flamePass;
    bool detonator;
} PlayerStats;

/* Player entity */
typedef struct {
    Entity base;
    PlayerStats stats;
    int lives;
    int deathTimer;      /* >0 while death animation plays */
    bool dead;
} Player;

/* ------------------------------------------------------------------ */
/* Player API                                                          */
/* ------------------------------------------------------------------ */

void player_init(Player *p, int col, int row);

/* Reposition the player without resetting lives/power-ups.           */
void player_place(Player *p, int col, int row);
void player_reset_stats(Player *p);

/* Update movement. `inputX/inputY` are -1,0,1 from the controller.    */
/* `bombs` is queried for bomb solidity.                               */
void player_update(Player *p, const GameMap *map, const BombSystem *bombs,
                   int inputX, int inputY);

/* Apply a collected power-up. Returns true when applied.              */
bool player_apply_item(Player *p, ItemType item);

/* Pixel-space AABB used for collision tests.                          */
void player_aabb(const Player *p, float *x0, float *y0, float *x1, float *y1);

#endif /* PLAYER_H */