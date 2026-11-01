#include "map.h"
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Stage layouts (index 0 = classic NES-like arena)                    */
/* 1 = fixed wall, 2 = soft brick, 0 = empty                           */
/* ------------------------------------------------------------------ */
static const int stageLayouts[MAX_STAGES][MAP_ROWS][MAP_COLS] = {
    /* Stage 1 - classic arena */
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    /* Stage 2 - more open with central pillar */
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

/* Corners that must stay clear for player/enemy spawns */
static bool isReservedCell(int row, int col) {
    if ((row <= 2 && col <= 2) || (row <= 2 && col >= MAP_COLS - 3) ||
        (row >= MAP_ROWS - 3 && col <= 2)) {
        return true;
    }
    if ((row == MAP_ROWS - 2 && col == MAP_COLS - 2) ||
        (row == 1 && col == MAP_COLS - 2) ||
        (row == MAP_ROWS - 2 && col == 1)) {
        return true;
    }
    return false;
}

void map_load(GameMap *map, int stageIndex) {
    if (stageIndex < 0) stageIndex = 0;
    if (stageIndex >= MAX_STAGES) stageIndex = (stageIndex % MAX_STAGES);

    /* Copy the fixed wall skeleton of the chosen stage */
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            map->cells[row][col] = stageLayouts[stageIndex][row][col];
        }
    }

    /* Scatter soft bricks on empty floor, keeping spawn corners free */
    for (int row = 1; row < MAP_ROWS - 1; row++) {
        for (int col = 1; col < MAP_COLS - 1; col++) {
            if (map->cells[row][col] == CELL_EMPTY && !isReservedCell(row, col)) {
                if (rand() % 100 < 55) {
                    map->cells[row][col] = CELL_BRICK;
                }
            }
        }
    }
}

bool map_is_wall(const GameMap *map, int col, int row) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return true;
    return map->cells[row][col] == CELL_WALL;
}

bool map_is_empty(const GameMap *map, int col, int row) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
    return map->cells[row][col] == CELL_EMPTY;
}

bool map_cell_solid(const GameMap *map, int col, int row, bool wallPass) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return true;
    int cell = map->cells[row][col];
    if (cell == CELL_WALL) return true;
    if (cell == CELL_BRICK && !wallPass) return true;
    return false;
}

bool map_break_brick(GameMap *map, int col, int row) {
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
    if (map->cells[row][col] != CELL_BRICK) return false;
    map->cells[row][col] = CELL_EMPTY;
    return true;
}