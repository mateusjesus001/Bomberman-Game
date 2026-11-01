#ifndef BOMB_H
#define BOMB_H

#include "bomber.h"

/* Bomb entity */
typedef struct {
    bool active;
    int col, row;        /* grid position          */
    int timer;           /* frames until explosion */
    int phaseTimer;      /* blink animation timer  */
    bool counting;       /* true when fuse burning */
    bool removable;      /* can be picked up (future) */
} Bomb;

/* Bomb system (pool) */
typedef struct {
    Bomb pool[MAX_BOMBS];
    int count;           /* active bombs */
} BombSystem;

/* ------------------------------------------------------------------ */
/* Bomb API                                                            */
/* ------------------------------------------------------------------ */

void bomb_system_init(BombSystem *bs);

/* Place a bomb at (col,row). Returns true when placed.               */
bool bomb_place(BombSystem *bs, int col, int row);

/* Returns true if a bomb occupies (col,row).                         */
bool bomb_at(const BombSystem *bs, int col, int row);

/* Returns true if a bomb at (col,row) was just placed (pass-through).*/
bool bomb_just_placed(const BombSystem *bs, int col, int row);

/* Returns true if a bomb at (col,row) is solid for the player.       */
/* `playerCol/Row` is the player anchor cell.                         */
/* `playerBombPass` reflects the Bomb Pass power-up.                  */
bool bomb_is_solid_for_player(const BombSystem *bs, int col, int row,
                              int playerCol, int playerRow, bool playerBombPass);

/* Count active bombs.                                                */
int bomb_count(const BombSystem *bs);

/* Detonate every bomb (used by Detonator item).                      */
void bomb_detonate_all(BombSystem *bs);

/* Advance bomb timers. Returns a list of bombs that exploded.        */
/* Caller is responsible for spawning explosions.                     */
int bomb_update(BombSystem *bs, int *explodedIdx, int *explodedCol, int *explodedRow, int maxList);

/* Remove a bomb by index.                                            */
void bomb_remove(BombSystem *bs, int index);

#endif /* BOMB_H */