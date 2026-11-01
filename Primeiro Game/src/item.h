#ifndef ITEM_H
#define ITEM_H

#include "bomber.h"
#include "map.h"

/* Item entity */
typedef struct {
    bool active;
    int col, row;
    ItemType type;
    int blinkTimer;      /* blink animation */
    bool revealed;       /* true when visible */
} Item;

/* Item system (pool) */
typedef struct {
    Item pool[MAX_ITEMS];
    int count;
} ItemSystem;

/* ------------------------------------------------------------------ */
/* Item API                                                            */
/* ------------------------------------------------------------------ */

void item_system_init(ItemSystem *is);

/* Spawn a random power-up at (col,row). Returns true when spawned.   */
bool item_spawn_random(ItemSystem *is, int col, int row);

/* Spawn a specific power-up at (col,row).                            */
bool item_spawn(ItemSystem *is, int col, int row, ItemType type);

/* Returns the item at (col,row), or NULL.                            */
Item *item_at(ItemSystem *is, int col, int row);

/* Remove an item by pointer.                                         */
void item_remove(ItemSystem *is, Item *item);

/* Advance blink timers.                                              */
void item_update(ItemSystem *is);

#endif /* ITEM_H */