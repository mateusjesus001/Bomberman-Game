#include "player.h"
#include <math.h>

/* ------------------------------------------------------------------ */
/* Speed table (pixels per frame at 60 fps)                            */
/* ------------------------------------------------------------------ */
static const float SPEED_TABLE[5] = {
    2.2f,   /* level 0 - slow   */
    2.8f,   /* level 1           */
    3.4f,   /* level 2 - default */
    4.0f,   /* level 3           */
    4.6f    /* level 4 - fast    */
};

/* Half of the player's collision box (pixels) */
#define PLAYER_HALF 14.0f

/* Margin used to decide when the player is "aligned" to a cell center */
#define ALIGN_MARGIN 2.0f

void player_place(Player *p, int col, int row) {
    p->base.x = COL_TO_PX(col);
    p->base.y = ROW_TO_PY(row);
    p->base.col = col;
    p->base.row = row;
    p->base.facing = DIR_RIGHT;
    p->base.moving = false;
    p->base.alive = true;
    p->base.touching = false;
    p->deathTimer = 0;
    p->dead = false;
}

void player_init(Player *p, int col, int row) {
    player_place(p, col, row);
    p->lives = 3;
    player_reset_stats(p);
}

void player_reset_stats(Player *p) {
    p->stats.maxBombs = 1;
    p->stats.fireRadius = 2;
    p->stats.speedLevel = 2;
    p->stats.wallPass = false;
    p->stats.bombPass = false;
    p->stats.flamePass = false;
    p->stats.detonator = false;
}

bool player_apply_item(Player *p, ItemType item) {
    switch (item) {
        case ITEM_EXTRA_BOMB:
            if (p->stats.maxBombs < MAX_BOMBS) {
                p->stats.maxBombs++;
                return true;
            }
            return false;
        case ITEM_FIRE_UP:
            if (p->stats.fireRadius < 8) {
                p->stats.fireRadius++;
                return true;
            }
            return false;
        case ITEM_SPEED_UP:
            if (p->stats.speedLevel < 4) {
                p->stats.speedLevel++;
                return true;
            }
            return false;
        case ITEM_WALL_PASS:
            p->stats.wallPass = true;
            return true;
        case ITEM_BOMB_PASS:
            p->stats.bombPass = true;
            return true;
        case ITEM_FLAME_PASS:
            p->stats.flamePass = true;
            return true;
        case ITEM_DETONATOR:
            p->stats.detonator = true;
            return true;
        default:
            return false;
    }
}

void player_aabb(const Player *p, float *x0, float *y0, float *x1, float *y1) {
    float cx = p->base.x + TILE_SIZE / 2.0f;
    float cy = p->base.y + TILE_SIZE / 2.0f;
    *x0 = cx - PLAYER_HALF;
    *y0 = cy - PLAYER_HALF;
    *x1 = cx + PLAYER_HALF;
    *y1 = cy + PLAYER_HALF;
}

/* ------------------------------------------------------------------ */
/* Collision helpers                                                   */
/* ------------------------------------------------------------------ */

/* Is the cell blocked for the player?                                 */
static bool cellBlocked(const Player *p, const GameMap *map, const BombSystem *bombs,
                        int col, int row) {
    if (map_cell_solid(map, col, row, p->stats.wallPass)) return true;
    if (bomb_is_solid_for_player(bombs, col, row,
                                 p->base.col, p->base.row, p->stats.bombPass)) {
        return true;
    }
    return false;
}

/* ------------------------------------------------------------------ */
/* Movement                                                            */
/* ------------------------------------------------------------------ */

void player_update(Player *p, const GameMap *map, const BombSystem *bombs,
                   int inputX, int inputY) {
    if (p->dead) return;

    float speed = SPEED_TABLE[p->stats.speedLevel];

    /* Determine desired direction (no diagonals) */
    int dirX = 0, dirY = 0;
    if (inputX != 0 && inputY == 0) {
        dirX = inputX;
    } else if (inputY != 0 && inputX == 0) {
        dirY = inputY;
    }

    /* Update facing */
    if (dirX > 0) p->base.facing = DIR_RIGHT;
    else if (dirX < 0) p->base.facing = DIR_LEFT;
    else if (dirY > 0) p->base.facing = DIR_DOWN;
    else if (dirY < 0) p->base.facing = DIR_UP;

    p->base.moving = (dirX != 0 || dirY != 0);

    if (!p->base.moving) return;

    /* ------------------------------------------------------------------ */
    /* Corner correction: when moving horizontally and blocked, try to    */
    /* slide vertically into an open corridor (and vice-versa).           */
    /* ------------------------------------------------------------------ */
    if (dirX != 0) {
        /* Which row band is the player currently in? */
        float centerY = p->base.y + TILE_SIZE / 2.0f;
        int row = (int)((centerY - MAP_OFFSET_Y) / TILE_SIZE);
        float rowCenter = ROW_TO_PY(row) + TILE_SIZE / 2.0f;

        /* If we are near the center of a row, try corner correction */
        if (fabsf(centerY - rowCenter) < ALIGN_MARGIN) {
            int targetCol = p->base.col + dirX;
            if (cellBlocked(p, map, bombs, targetCol, row)) {
                /* Try to slide up or down */
                int slideRow = row - 1;
                if (slideRow >= 0 && !cellBlocked(p, map, bombs, p->base.col, slideRow)) {
                    p->base.y -= speed;
                    p->base.row = slideRow;
                    return;
                }
                slideRow = row + 1;
                if (slideRow < MAP_ROWS && !cellBlocked(p, map, bombs, p->base.col, slideRow)) {
                    p->base.y += speed;
                    p->base.row = slideRow;
                    return;
                }
                /* Fully blocked - stop */
                return;
            }
        }
    } else if (dirY != 0) {
        /* Which column band is the player currently in? */
        float centerX = p->base.x + TILE_SIZE / 2.0f;
        int col = (int)((centerX - MAP_OFFSET_X) / TILE_SIZE);
        float colCenter = COL_TO_PX(col) + TILE_SIZE / 2.0f;

        if (fabsf(centerX - colCenter) < ALIGN_MARGIN) {
            int targetRow = p->base.row + dirY;
            if (cellBlocked(p, map, bombs, col, targetRow)) {
                /* Try to slide left or right */
                int slideCol = col - 1;
                if (slideCol >= 0 && !cellBlocked(p, map, bombs, slideCol, p->base.row)) {
                    p->base.x -= speed;
                    p->base.col = slideCol;
                    return;
                }
                slideCol = col + 1;
                if (slideCol < MAP_COLS && !cellBlocked(p, map, bombs, slideCol, p->base.row)) {
                    p->base.x += speed;
                    p->base.col = slideCol;
                    return;
                }
                return;
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /* Normal movement                                                    */
    /* ------------------------------------------------------------------ */
    float newX = p->base.x;
    float newY = p->base.y;

    if (dirX != 0) {
        newX += dirX * speed;
    }
    if (dirY != 0) {
        newY += dirY * speed;
    }

    /* Horizontal movement */
    if (dirX != 0) {
        int checkCol = p->base.col + dirX;
        int checkRow = p->base.row;
        if (!cellBlocked(p, map, bombs, checkCol, checkRow)) {
            p->base.x = newX;
            p->base.col = (int)((newX + TILE_SIZE / 2.0f - MAP_OFFSET_X) / TILE_SIZE);
        } else {
            /* Snap to the edge of the blocking cell */
            if (dirX > 0) {
                p->base.x = COL_TO_PX(checkCol) - TILE_SIZE;
            } else {
                p->base.x = COL_TO_PX(checkCol) + TILE_SIZE;
            }
        }
    }

    /* Vertical movement */
    if (dirY != 0) {
        int checkCol = p->base.col;
        int checkRow = p->base.row + dirY;
        if (!cellBlocked(p, map, bombs, checkCol, checkRow)) {
            p->base.y = newY;
            p->base.row = (int)((newY + TILE_SIZE / 2.0f - MAP_OFFSET_Y) / TILE_SIZE);
        } else {
            if (dirY > 0) {
                p->base.y = ROW_TO_PY(checkRow) - TILE_SIZE;
            } else {
                p->base.y = ROW_TO_PY(checkRow) + TILE_SIZE;
            }
        }
    }

    /* Clamp to map bounds */
    if (p->base.x < MAP_OFFSET_X) p->base.x = MAP_OFFSET_X;
    if (p->base.y < MAP_OFFSET_Y) p->base.y = MAP_OFFSET_Y;
    if (p->base.x > MAP_OFFSET_X + (MAP_COLS - 1) * TILE_SIZE) {
        p->base.x = MAP_OFFSET_X + (MAP_COLS - 1) * TILE_SIZE;
    }
    if (p->base.y > MAP_OFFSET_Y + (MAP_ROWS - 1) * TILE_SIZE) {
        p->base.y = MAP_OFFSET_Y + (MAP_ROWS - 1) * TILE_SIZE;
    }

    /* Update anchor cell */
    p->base.col = (int)((p->base.x + TILE_SIZE / 2.0f - MAP_OFFSET_X) / TILE_SIZE);
    p->base.row = (int)((p->base.y + TILE_SIZE / 2.0f - MAP_OFFSET_Y) / TILE_SIZE);
}