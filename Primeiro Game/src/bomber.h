#ifndef BOMBER_H
#define BOMBER_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Window / Grid configuration                                         */
/* ------------------------------------------------------------------ */
#define WINDOW_W       640
#define WINDOW_H       568
#define HUD_HEIGHT     48
#define MAP_OFFSET_X   20
#define MAP_OFFSET_Y   48

#define TILE_SIZE      40
#define MAP_COLS       15
#define MAP_ROWS       13

/* ------------------------------------------------------------------ */
/* Entity limits                                                       */
/* ------------------------------------------------------------------ */
#define MAX_ENEMIES       8
#define MAX_BOMBS         8
#define MAX_EXPLOSIONS    64
#define MAX_ITEMS         32
#define MAX_STAGES        8

/* ------------------------------------------------------------------ */
/* Timing (in frames at 60 fps)                                        */
/* ------------------------------------------------------------------ */
#define BOMB_TIMER_FRAMES      120   /* 2.0s  - NES classic feel  */
#define EXPLOSION_FRAMES       22    /* ~0.37s                     */
#define EXPLOSION_FORK_FRAMES  70    /* fork stays slightly longer */
#define ENEMY_WALK_FRAMES      40    /* ~0.67s per tile            */
#define PLAYER_DEATH_FRAMES    90    /* ~1.5s death animation      */

/* ------------------------------------------------------------------ */
/* Map cell values                                                     */
/* ------------------------------------------------------------------ */
#define CELL_EMPTY     0
#define CELL_WALL      1
#define CELL_BRICK     2

/* ------------------------------------------------------------------ */
/* Power-up identifiers                                                */
/* ------------------------------------------------------------------ */
typedef enum {
    ITEM_NONE,
    ITEM_EXTRA_BOMB,     /* increases bomb capacity            */
    ITEM_FIRE_UP,        /* increases explosion radius         */
    ITEM_SPEED_UP,       /* increases walk speed               */
    ITEM_WALL_PASS,      /* walk through soft bricks           */
    ITEM_BOMB_PASS,      /* walk through bombs                 */
    ITEM_FLAME_PASS,     /* walk through fire safely           */
    ITEM_DETONATOR,      /* remote bomb detonation             */
    ITEM_ITEM_COUNT
} ItemType;

/* ------------------------------------------------------------------ */
/* Direction constants                                                 */
/* ------------------------------------------------------------------ */
enum {
    DIR_RIGHT = 0,
    DIR_LEFT  = 1,
    DIR_DOWN  = 2,
    DIR_UP    = 3,
    DIR_NONE  = 4
};

/* ------------------------------------------------------------------ */
/* Shared types                                                        */
/* ------------------------------------------------------------------ */
typedef int  TileCoord;   /* column or row (grid space) */
typedef float PxCoord;    /* pixel position (world space) */

typedef struct {
    PxCoord x, y;         /* top-left pixel position            */
    int col, row;         /* current grid cell (anchor)         */
    int facing;           /* DIR_*                              */
    bool moving;
    bool alive;
    bool touching;        /* controller state */
} Entity;

/* Grid helper: pixel -> cell */
#define PX_TO_COL(px)  ((int)(((px) - MAP_OFFSET_X) / TILE_SIZE))
#define PX_TO_ROW(py)  ((int)(((py) - MAP_OFFSET_Y) / TILE_SIZE))

/* Cell helper: cell -> pixel */
#define COL_TO_PX(col) ((PxCoord)(MAP_OFFSET_X + (col) * TILE_SIZE))
#define ROW_TO_PY(row) ((PxCoord)(MAP_OFFSET_Y + (row) * TILE_SIZE))

#endif /* BOMBER_H */