#ifndef MAP_H
#define MAP_H

#include "bomber.h"

/* Ensure every spawn corner stays clear of bricks. */
typedef struct {
    int col, row;
} SpawnPoint;

/* Public map state */
typedef struct {
    int cells[MAP_ROWS][MAP_COLS];
} GameMap;

/* ------------------------------------------------------------------ */
/* Map API                                                             */
/* ------------------------------------------------------------------ */

/* Build a stage layout. `stageIndex` is 0-based. */
void map_load(GameMap *map, int stageIndex);

/* True when pixel-space entity AABB overlaps the tile at (col,row).   */
bool map_cell_solid(const GameMap *map, int col, int row, bool wallPass);

/* Destroy a soft brick at (col,row). Returns true when destroyed.     */
bool map_break_brick(GameMap *map, int col, int row);

/* Is the cell a permanent wall (never destructible)?                  */
bool map_is_wall(const GameMap *map, int col, int row);

/* Is the cell an empty floor tile?                                    */
bool map_is_empty(const GameMap *map, int col, int row);

#endif /* MAP_H */