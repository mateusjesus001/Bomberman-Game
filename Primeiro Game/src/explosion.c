#include "explosion.h"

/* Explosion segment types */
#define SEG_CENTER 0
#define SEG_HORIZ  1
#define SEG_VERT   2
#define SEG_END    3

void explosion_system_init(ExplosionSystem *es) {
    es->count = 0;
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        es->pool[i].active = false;
        es->pool[i].timer = 0;
        es->pool[i].type = SEG_CENTER;
    }
}

static void addSegment(ExplosionSystem *es, int col, int row, int type) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!es->pool[i].active) {
            es->pool[i].active = true;
            es->pool[i].col = col;
            es->pool[i].row = row;
            es->pool[i].timer = EXPLOSION_FRAMES;
            es->pool[i].type = type;
            es->count++;
            return;
        }
    }
}

int explosion_spawn(ExplosionSystem *es, GameMap *map, BombSystem *bs,
                    int col, int row, int radius,
                    int destroyedBricks[][2], int *destroyedCount) {
    int chainCount = 0;

    /* Center segment */
    addSegment(es, col, row, SEG_CENTER);

    /* Chain reaction: any bomb at the center explodes immediately */
    if (bs) {
        for (int i = 0; i < MAX_BOMBS; i++) {
            if (bs->pool[i].active && bs->pool[i].col == col && bs->pool[i].row == row) {
                bs->pool[i].timer = 0;
                chainCount++;
            }
        }
    }

    /* Four directions */
    const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (int d = 0; d < 4; d++) {
        for (int dist = 1; dist <= radius; dist++) {
            int nx = col + dirs[d][0] * dist;
            int ny = row + dirs[d][1] * dist;

            /* Out of bounds or fixed wall: stop this direction */
            if (nx < 0 || nx >= MAP_COLS || ny < 0 || ny >= MAP_ROWS) break;
            if (map->cells[ny][nx] == CELL_WALL) break;

            /* Soft brick: destroy it and stop */
            if (map->cells[ny][nx] == CELL_BRICK) {
                map->cells[ny][nx] = CELL_EMPTY;
                addSegment(es, nx, ny, SEG_END);
                /* Record destroyed brick for item spawning */
                if (destroyedBricks && destroyedCount) {
                    if (*destroyedCount < 64) {
                        destroyedBricks[*destroyedCount][0] = nx;
                        destroyedBricks[*destroyedCount][1] = ny;
                        (*destroyedCount)++;
                    }
                }
                break;
            }

            /* Determine segment type */
            int type = (dirs[d][0] != 0) ? SEG_HORIZ : SEG_VERT;
            if (dist == radius) type = SEG_END;

            addSegment(es, nx, ny, type);

            /* Chain reaction: bomb hit by fire explodes immediately */
            if (bs) {
                for (int i = 0; i < MAX_BOMBS; i++) {
                    if (bs->pool[i].active && bs->pool[i].col == nx && bs->pool[i].row == ny) {
                        bs->pool[i].timer = 0;
                        chainCount++;
                    }
                }
            }
        }
    }

    return chainCount;
}

bool explosion_at(const ExplosionSystem *es, int col, int row) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (es->pool[i].active && es->pool[i].col == col && es->pool[i].row == row) {
            return true;
        }
    }
    return false;
}

void explosion_update(ExplosionSystem *es) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (es->pool[i].active) {
            es->pool[i].timer--;
            if (es->pool[i].timer <= 0) {
                es->pool[i].active = false;
                es->count--;
            }
        }
    }
}

bool explosion_hits(const ExplosionSystem *es, int col, int row, bool flamePass) {
    if (flamePass) return false;
    return explosion_at(es, col, row);
}