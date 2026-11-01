#ifndef ENEMY_H
#define ENEMY_H

#include "bomber.h"
#include "map.h"
#include "bomb.h"
#include "explosion.h"

/* Enemy types (NES-inspired) */
typedef enum {
    ENEMY_BALLOON,   /* slow, wanders randomly */
    ENEMY_ONION,     /* medium speed, chases player */
    ENEMY_DOLL,      /* fast, erratic */
    ENEMY_COUNT
} EnemyType;

/* Enemy states */
typedef enum {
    ENEMY_STATE_WANDER,
    ENEMY_STATE_CHASE,
    ENEMY_STATE_DYING
} EnemyState;

/* Enemy entity */
typedef struct {
    Entity base;
    EnemyType type;
    EnemyState state;
    int moveTimer;       /* frames until next tile decision */
    int animTimer;       /* animation frame */
    int deathTimer;      /* death animation */
    bool alive;
} Enemy;

/* Enemy system */
typedef struct {
    Enemy pool[MAX_ENEMIES];
    int count;
} EnemySystem;

/* ------------------------------------------------------------------ */
/* Enemy API                                                           */
/* ------------------------------------------------------------------ */

void enemy_system_init(EnemySystem *es);

/* Spawn an enemy at (col,row) with given type.                       */
bool enemy_spawn(EnemySystem *es, int col, int row, EnemyType type);

/* Update all enemies. `playerCol/Row` used for chase AI.              */
void enemy_update(EnemySystem *es, const GameMap *map, const BombSystem *bs,
                  const ExplosionSystem *ex, int playerCol, int playerRow);

/* Kill an enemy (start death animation). Returns true when killed.   */
bool enemy_kill(EnemySystem *es, int index);

/* Returns true if an enemy occupies (col,row).                       */
bool enemy_at(const EnemySystem *es, int col, int row);

/* Remove dead enemies from the pool.                                 */
void enemy_cleanup(EnemySystem *es);

#endif /* ENEMY_H */