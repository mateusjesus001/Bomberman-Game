#include "item.h"
#include <stdlib.h>

void item_system_init(ItemSystem *is) {
    is->count = 0;
    for (int i = 0; i < MAX_ITEMS; i++) {
        is->pool[i].active = false;
        is->pool[i].blinkTimer = 0;
        is->pool[i].revealed = false;
    }
}

bool item_spawn(ItemSystem *is, int col, int row, ItemType type) {
    if (is->count >= MAX_ITEMS) return false;
    /* Do not stack two items on the same cell */
    if (item_at(is, col, row) != NULL) return false;

    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!is->pool[i].active) {
            is->pool[i].active = true;
            is->pool[i].col = col;
            is->pool[i].row = row;
            is->pool[i].type = type;
            is->pool[i].blinkTimer = 0;
            is->pool[i].revealed = true;
            is->count++;
            return true;
        }
    }
    return false;
}

bool item_spawn_random(ItemSystem *is, int col, int row) {
    /* Weighted random: common items appear more often */
    int roll = rand() % 100;
    ItemType type;
    if (roll < 30) {
        type = ITEM_EXTRA_BOMB;
    } else if (roll < 55) {
        type = ITEM_FIRE_UP;
    } else if (roll < 75) {
        type = ITEM_SPEED_UP;
    } else if (roll < 85) {
        type = ITEM_WALL_PASS;
    } else if (roll < 93) {
        type = ITEM_BOMB_PASS;
    } else if (roll < 98) {
        type = ITEM_FLAME_PASS;
    } else {
        type = ITEM_DETONATOR;
    }
    return item_spawn(is, col, row, type);
}

Item *item_at(ItemSystem *is, int col, int row) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (is->pool[i].active && is->pool[i].col == col && is->pool[i].row == row) {
            return &is->pool[i];
        }
    }
    return NULL;
}

void item_remove(ItemSystem *is, Item *item) {
    if (!item || !item->active) return;
    item->active = false;
    is->count--;
}

void item_update(ItemSystem *is) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (is->pool[i].active) {
            is->pool[i].blinkTimer++;
        }
    }
}