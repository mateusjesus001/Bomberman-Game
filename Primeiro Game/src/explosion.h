#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "bomber.h"
#include "map.h"
#include "bomb.h"

/* Explosion segment */
typedef struct {
    bool active;
    int col, row;
    int timer;
    int type;        /* 0=center, 1=horizontal, 2=vertical, 3=end */
} Explosion;

/* Explosion system (pool) */
typedef struct {
    Explosion pool[MAX_EXPLOSIONS];
    int count;
} ExplosionSystem;

/* ------------------------------------------------------------------ */
/* Explosion API                                                       */
/* ------------------------------------------------------------------ */

void explosion_system_init(ExplosionSystem *es);

/* Spawn a full cross explosion at (col,row) with given radius.        */
/* `map` is used to stop at walls and destroy bricks.                  */
/* `bombs` is used for chain reaction (bombs hit by fire explode).     */
/* `destroyedBricks` (optional) receives positions of bricks destroyed.*/
/* Returns number of bombs that were chain-detonated.                  */
int explosion_spawn(ExplosionSystem *es, GameMap *map, BombSystem *bs,
                    int col, int row, int radius,
                    int destroyedBricks[][2], int *destroyedCount);

/* Returns true if an active explosion covers (col,row).               */
bool explosion_at(const ExplosionSystem *es, int col, int row);

/* Advance timers, removing expired explosions.                        */
void explosion_update(ExplosionSystem *es);

/* Returns true if the player (with flamePass) is safe from fire.      */
bool explosion_hits(const ExplosionSystem *es, int col, int row, bool flamePass);

#endif /* EXPLOSION_H */