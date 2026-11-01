#ifndef RENDER_H
#define RENDER_H

#include "bomber.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "bomb.h"
#include "explosion.h"
#include "item.h"

/* Door state */
typedef struct {
    bool active;
    bool revealed;
    int col, row;
    int animTimer;
} Door;

/* Sprite wrapper: BMP loaded as texture with dimensions */
typedef struct {
    SDL_Texture *texture;
    int srcW, srcH;   /* original sprite pixel size */
    int dstW, dstH;   /* target draw size           */
} Sprite;

/* ------------------------------------------------------------------ */
/* Render API                                                          */
/* ------------------------------------------------------------------ */

/* Load BMP sprites into textures. Call once after renderer creation.
   Returns true on success. */
bool render_init(SDL_Renderer *renderer);

/* Free loaded sprite textures. */
void render_quit(void);

void render_map(SDL_Renderer *renderer, const GameMap *map);
void render_player(SDL_Renderer *renderer, const Player *player);
void render_enemies(SDL_Renderer *renderer, const EnemySystem *es);
void render_bombs(SDL_Renderer *renderer, const BombSystem *bs);
void render_explosions(SDL_Renderer *renderer, const ExplosionSystem *es);
void render_items(SDL_Renderer *renderer, const ItemSystem *is);
void render_door(SDL_Renderer *renderer, const Door *door);

#endif /* RENDER_H */