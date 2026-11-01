#include "bomb.h"

/* Frames during which the player can walk through a freshly placed bomb */
#define BOMB_PASS_FRAMES 20

void bomb_system_init(BombSystem *bs) {
    bs->count = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        bs->pool[i].active = false;
        bs->pool[i].timer = 0;
        bs->pool[i].phaseTimer = 0;
        bs->pool[i].counting = false;
        bs->pool[i].removable = false;
    }
}

bool bomb_place(BombSystem *bs, int col, int row) {
    if (bs->count >= MAX_BOMBS) return false;
    /* Do not stack two bombs on the same cell */
    if (bomb_at(bs, col, row)) return false;

    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bs->pool[i].active) {
            bs->pool[i].active = true;
            bs->pool[i].col = col;
            bs->pool[i].row = row;
            bs->pool[i].timer = BOMB_TIMER_FRAMES;
            bs->pool[i].phaseTimer = 0;
            bs->pool[i].counting = true;
            bs->pool[i].removable = false;
            bs->count++;
            return true;
        }
    }
    return false;
}

bool bomb_at(const BombSystem *bs, int col, int row) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bs->pool[i].active && bs->pool[i].col == col && bs->pool[i].row == row) {
            return true;
        }
    }
    return false;
}

bool bomb_just_placed(const BombSystem *bs, int col, int row) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bs->pool[i].active && bs->pool[i].col == col && bs->pool[i].row == row) {
            /* Freshly placed bombs are walkable for a short window */
            return bs->pool[i].timer > BOMB_TIMER_FRAMES - BOMB_PASS_FRAMES;
        }
    }
    return false;
}

bool bomb_is_solid_for_player(const BombSystem *bs, int col, int row,
                              int playerCol, int playerRow, bool playerBombPass) {
    if (!bomb_at(bs, col, row)) return false;
    if (playerBombPass) return false;

    /* The bomb under the player is only passable immediately after placing */
    if (col == playerCol && row == playerRow) {
        return !bomb_just_placed(bs, col, row);
    }
    return true;
}

int bomb_count(const BombSystem *bs) {
    return bs->count;
}

void bomb_detonate_all(BombSystem *bs) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (bs->pool[i].active) {
            bs->pool[i].timer = 0; /* explode next update */
        }
    }
}

int bomb_update(BombSystem *bs, int *explodedIdx, int *explodedCol, int *explodedRow, int maxList) {
    int n = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bs->pool[i].active) continue;
        if (bs->pool[i].counting) {
            bs->pool[i].timer--;
            bs->pool[i].phaseTimer++;
            if (bs->pool[i].timer <= 0) {
                if (n < maxList) {
                    explodedIdx[n] = i;
                    explodedCol[n] = bs->pool[i].col;
                    explodedRow[n] = bs->pool[i].row;
                    n++;
                }
            }
        }
    }
    return n;
}

void bomb_remove(BombSystem *bs, int index) {
    if (index < 0 || index >= MAX_BOMBS) return;
    if (bs->pool[index].active) {
        bs->pool[index].active = false;
        bs->count--;
    }
}