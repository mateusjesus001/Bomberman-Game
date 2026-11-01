#include "enemy.h"
#include <stdlib.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Per-type speed (pixels/frame) and decision rate                     */
/* ------------------------------------------------------------------ */
typedef struct {
    float speed;
    int   decisionFrames;   /* frames between tile decisions */
} EnemyProfile;

static const EnemyProfile PROFILES[ENEMY_COUNT] = {
    { 1.8f, 45 },   /* BALLOON - slow   */
    { 2.6f, 30 },   /* ONION   - medium */
    { 3.4f, 20 }    /* DOLL    - fast, erratic */
};

void enemy_system_init(EnemySystem *es) {
    es->count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        es->pool[i].alive = false;
    }
}

bool enemy_spawn(EnemySystem *es, int col, int row, EnemyType type) {
    if (es->count >= MAX_ENEMIES) return false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!es->pool[i].alive) {
            es->pool[i].type = type;
            es->pool[i].base.x = COL_TO_PX(col);
            es->pool[i].base.y = ROW_TO_PY(row);
            es->pool[i].base.col = col;
            es->pool[i].base.row = row;
            es->pool[i].base.facing = DIR_RIGHT;
            es->pool[i].base.moving = true;
            es->pool[i].state = ENEMY_STATE_WANDER;
            es->pool[i].moveTimer = rand() % PROFILES[type].decisionFrames;
            es->pool[i].animTimer = 0;
            es->pool[i].deathTimer = 0;
            es->pool[i].alive = true;
            es->count++;
            return true;
        }
    }
    return false;
}

bool enemy_at(const EnemySystem *es, int col, int row) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (es->pool[i].alive && es->pool[i].state != ENEMY_STATE_DYING &&
            es->pool[i].base.col == col && es->pool[i].base.row == row) {
            return true;
        }
    }
    return false;
}

bool enemy_kill(EnemySystem *es, int index) {
    if (index < 0 || index >= MAX_ENEMIES) return false;
    Enemy *e = &es->pool[index];
    if (!e->alive || e->state == ENEMY_STATE_DYING) return false;
    e->state = ENEMY_STATE_DYING;
    e->deathTimer = 30;   /* death animation length */
    return true;
}

void enemy_cleanup(EnemySystem *es) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (es->pool[i].alive && es->pool[i].state == ENEMY_STATE_DYING) {
            es->pool[i].deathTimer--;
            if (es->pool[i].deathTimer <= 0) {
                es->pool[i].alive = false;
                es->count--;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Movement helpers                                                    */
/* ------------------------------------------------------------------ */

static bool cellWalkable(const GameMap *map, const BombSystem *bs,
                         const ExplosionSystem *ex, int col, int row) {
    if (map_cell_solid(map, col, row, false)) return false;
    if (bomb_at(bs, col, row)) return false;
    if (explosion_at(ex, col, row)) return false;
    return true;
}

/* Choose a new random direction when blocked or on decision tick.     */
static int chooseDirection(Enemy *e, const GameMap *map, const BombSystem *bs,
                           const ExplosionSystem *ex, int playerCol, int playerRow) {
    const int dirs[4] = { DIR_RIGHT, DIR_LEFT, DIR_DOWN, DIR_UP };
    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    /* Chase AI: ONION moves toward the player when aligned */
    if (e->type == ENEMY_ONION && e->state == ENEMY_STATE_CHASE) {
        int bestDir = -1;
        int bestDist = 9999;
        for (int d = 0; d < 4; d++) {
            int nx = e->base.col + dx[d];
            int ny = e->base.row + dy[d];
            if (!cellWalkable(map, bs, ex, nx, ny)) continue;
            int dist = abs(nx - playerCol) + abs(ny - playerRow);
            if (dist < bestDist) {
                bestDist = dist;
                bestDir = d;
            }
        }
        if (bestDir >= 0 && (rand() % 100) < 70) return bestDir;
    }

    /* Avoid reversing unless necessary */
    int candidates[4];
    int n = 0;
    for (int d = 0; d < 4; d++) {
        if (dirs[d] == e->base.facing) continue;
        int nx = e->base.col + dx[d];
        int ny = e->base.row + dy[d];
        if (cellWalkable(map, bs, ex, nx, ny)) {
            candidates[n++] = dirs[d];
        }
    }

    /* Blocked everywhere (corner) -> allow reverse */
    if (n == 0) {
        for (int d = 0; d < 4; d++) {
            int nx = e->base.col + dx[d];
            int ny = e->base.row + dy[d];
            if (cellWalkable(map, bs, ex, nx, ny)) {
                candidates[n++] = dirs[d];
            }
        }
    }

    if (n == 0) return e->base.facing;
    return candidates[rand() % n];
}

/* ------------------------------------------------------------------ */
/* Update                                                              */
/* ------------------------------------------------------------------ */

void enemy_update(EnemySystem *es, const GameMap *map, const BombSystem *bs,
                  const ExplosionSystem *ex, int playerCol, int playerRow) {
    const int dx[4] = { 1, -1, 0, 0 };
    const int dy[4] = { 0, 0, 1, -1 };

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &es->pool[i];
        if (!e->alive) continue;

        if (e->state == ENEMY_STATE_DYING) continue; /* handled by cleanup */

        e->animTimer++;
        const EnemyProfile *prof = &PROFILES[e->type];

        /* Decide new direction on a timer */
        e->moveTimer--;
        if (e->moveTimer <= 0) {
            e->moveTimer = prof->decisionFrames;

            /* State transitions */
            if (e->type == ENEMY_ONION) {
                int dist = abs(e->base.col - playerCol) + abs(e->base.row - playerRow);
                e->state = (dist <= 5) ? ENEMY_STATE_CHASE : ENEMY_STATE_WANDER;
            } else if (e->type == ENEMY_DOLL) {
                /* Erratic: always redecide */
            }

            e->base.facing = chooseDirection(e, map, bs, ex, playerCol, playerRow);
        }

        /* Move continuously in the chosen direction */
        float speed = prof->speed;
        float nx = e->base.x;
        float ny = e->base.y;

        if (e->base.facing == DIR_RIGHT) nx += speed;
        else if (e->base.facing == DIR_LEFT) nx -= speed;
        else if (e->base.facing == DIR_DOWN) ny += speed;
        else if (e->base.facing == DIR_UP) ny -= speed;

        /* Check the cell we are moving into */
        int targetCol = e->base.col;
        int targetRow = e->base.row;
        if (e->base.facing == DIR_RIGHT || e->base.facing == DIR_LEFT) {
            targetCol = e->base.col + ((e->base.facing == DIR_RIGHT) ? 1 : -1);
        } else {
            targetRow = e->base.row + ((e->base.facing == DIR_DOWN) ? 1 : -1);
        }

        if (cellWalkable(map, bs, ex, targetCol, targetRow)) {
            e->base.x = nx;
            e->base.y = ny;
        } else {
            /* Blocked: snap to the cell boundary and pick a new direction */
            if (e->base.facing == DIR_RIGHT) e->base.x = COL_TO_PX(targetCol) - TILE_SIZE;
            else if (e->base.facing == DIR_LEFT) e->base.x = COL_TO_PX(targetCol) + TILE_SIZE;
            else if (e->base.facing == DIR_DOWN) e->base.y = ROW_TO_PY(targetRow) - TILE_SIZE;
            else if (e->base.facing == DIR_UP) e->base.y = ROW_TO_PY(targetRow) + TILE_SIZE;

            e->moveTimer = 0; /* force immediate re-decision */
            e->base.facing = chooseDirection(e, map, bs, ex, playerCol, playerRow);
        }

        /* Keep inside the map */
        if (e->base.x < MAP_OFFSET_X) e->base.x = MAP_OFFSET_X;
        if (e->base.y < MAP_OFFSET_Y) e->base.y = MAP_OFFSET_Y;
        if (e->base.x > MAP_OFFSET_X + (MAP_COLS - 1) * TILE_SIZE) {
            e->base.x = MAP_OFFSET_X + (MAP_COLS - 1) * TILE_SIZE;
        }
        if (e->base.y > MAP_OFFSET_Y + (MAP_ROWS - 1) * TILE_SIZE) {
            e->base.y = MAP_OFFSET_Y + (MAP_ROWS - 1) * TILE_SIZE;
        }

        /* Update anchor cell */
        e->base.col = (int)((e->base.x + TILE_SIZE / 2.0f - MAP_OFFSET_X) / TILE_SIZE);
        e->base.row = (int)((e->base.y + TILE_SIZE / 2.0f - MAP_OFFSET_Y) / TILE_SIZE);
    }
}