#ifndef GAME_H
#define GAME_H

#include "bomber.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "bomb.h"
#include "explosion.h"
#include "item.h"
#include "hud.h"
#include "render.h"

/* Game states */
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_VICTORY,
    GAME_STATE_GAMEOVER
} GameState;

/* Full game context */
typedef struct {
    GameMap map;
    Player player;
    EnemySystem enemies;
    BombSystem bombs;
    ExplosionSystem explosions;
    ItemSystem items;
    HudData hud;
    Door door;
    GameState state;
    int stage;
    int frame;
    bool running;
} Game;

/* ------------------------------------------------------------------ */
/* Game API                                                            */
/* ------------------------------------------------------------------ */

void game_init(Game *g);
void game_reset_stage(Game *g);
void game_update(Game *g, int inputX, int inputY, bool placeBomb, bool detonate);
void game_draw(SDL_Renderer *renderer, const Game *g);

#endif /* GAME_H */