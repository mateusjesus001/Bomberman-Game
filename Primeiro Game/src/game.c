#include "game.h"
#include "audio.h"
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Stage configuration                                                 */
/* ------------------------------------------------------------------ */
typedef struct {
    int enemyCount;
    EnemyType types[MAX_ENEMIES];
    int spawnCols[MAX_ENEMIES];
    int spawnRows[MAX_ENEMIES];
} StageConfig;

static const StageConfig STAGES[MAX_STAGES] = {
    { 4, { ENEMY_BALLOON, ENEMY_BALLOON, ENEMY_BALLOON, ENEMY_BALLOON },
      { 13, 13, 1, 7 }, { 1, 11, 11, 5 } },
    { 5, { ENEMY_BALLOON, ENEMY_BALLOON, ENEMY_ONION, ENEMY_BALLOON, ENEMY_ONION },
      { 13, 13, 1, 1, 7 }, { 1, 11, 11, 1, 5 } },
    { 6, { ENEMY_BALLOON, ENEMY_ONION, ENEMY_ONION, ENEMY_DOLL, ENEMY_BALLOON, ENEMY_DOLL },
      { 13, 13, 1, 1, 7, 7 }, { 1, 11, 11, 1, 5, 5 } }
};

/* ------------------------------------------------------------------ */
/* Init / reset                                                        */
/* ------------------------------------------------------------------ */
void game_init(Game *g) {
    g->state = GAME_STATE_MENU;
    g->stage = 0;
    g->frame = 0;
    g->running = true;
    player_init(&g->player, 1, 1);
    bomb_system_init(&g->bombs);
    explosion_system_init(&g->explosions);
    item_system_init(&g->items);
    enemy_system_init(&g->enemies);
    hud_init(&g->hud);
    g->door.active = false;
    g->door.revealed = false;
    g->door.col = 0;
    g->door.row = 0;
    g->door.animTimer = 0;
}

void game_reset_stage(Game *g) {
    int stage = g->stage;
    if (stage >= MAX_STAGES) stage = stage % MAX_STAGES;

    int savedScore = g->hud.score;   /* score persists across deaths */

    map_load(&g->map, stage);
    player_place(&g->player, 1, 1);
    /* NOTE: power-ups persist across stages (NES rule).              */
    /* On death, the caller explicitly calls player_reset_stats().     */
    bomb_system_init(&g->bombs);
    explosion_system_init(&g->explosions);
    item_system_init(&g->items);
    enemy_system_init(&g->enemies);
    hud_init(&g->hud);
    g->hud.score = savedScore;

    /* Spawn enemies per stage config */
    const StageConfig *cfg = &STAGES[stage];
    for (int i = 0; i < cfg->enemyCount; i++) {
        enemy_spawn(&g->enemies, cfg->spawnCols[i], cfg->spawnRows[i], cfg->types[i]);
    }

    /* Door hidden under a random brick */
    g->door.active = false;   /* only usable when all enemies die */
    g->door.revealed = false; /* only visible when brick above destroyed */
    g->door.animTimer = 0;
    /* Find a random brick to hide the door */
    int candidates[64][2];
    int n = 0;
    for (int row = 1; row < MAP_ROWS - 1; row++) {
        for (int col = 1; col < MAP_COLS - 1; col++) {
            if (g->map.cells[row][col] == CELL_BRICK) {
                candidates[n][0] = col;
                candidates[n][1] = row;
                n++;
            }
        }
    }
    if (n > 0) {
        int idx = rand() % n;
        g->door.col = candidates[idx][0];
        g->door.row = candidates[idx][1];
    } else {
        g->door.col = 7;
        g->door.row = 6;
    }

    g->state = GAME_STATE_PLAYING;
    g->frame = 0;
}

/* ------------------------------------------------------------------ */
/* Update                                                              */
/* ------------------------------------------------------------------ */
void game_update(Game *g, int inputX, int inputY, bool placeBomb, bool detonate) {
    g->frame++;

    if (g->state == GAME_STATE_PLAYING) {
        /* --- Death animation: freeze the world while the player dies --- */
        if (g->player.dead) {
            g->player.deathTimer--;
            if (g->player.deathTimer <= 0) {
                game_reset_stage(g);
            }
            return;
        }

        /* --- Player movement (bombs queried internally) --- */
        player_update(&g->player, &g->map, &g->bombs, inputX, inputY);

        /* --- Place bomb --- */
        if (placeBomb) {
            if (bomb_count(&g->bombs) < g->player.stats.maxBombs) {
                if (bomb_place(&g->bombs, g->player.base.col, g->player.base.row)) {
                    audio_play(SFX_BOMB);
                }
            }
        }

        /* --- Detonator --- */
        if (detonate && g->player.stats.detonator) {
            bomb_detonate_all(&g->bombs);
        }

        /* --- Bomb timers & explosions --- */
        int explodedIdx[MAX_BOMBS];
        int explodedCol[MAX_BOMBS];
        int explodedRow[MAX_BOMBS];
        int nExploded = bomb_update(&g->bombs, explodedIdx, explodedCol, explodedRow, MAX_BOMBS);

        /* Track destroyed bricks for item spawning */
        int destroyedBricks[64][2];
        int destroyedCount = 0;

        /* Process explosions (including chain reactions) */
        for (int i = 0; i < nExploded; i++) {
            bomb_remove(&g->bombs, explodedIdx[i]);
            explosion_spawn(&g->explosions, &g->map, &g->bombs,
                            explodedCol[i], explodedRow[i],
                            g->player.stats.fireRadius,
                            destroyedBricks, &destroyedCount);
            audio_play(SFX_EXPLOSION);
        }

        /* Chain reaction: bombs whose timer hit 0 during explosion_spawn */
        /* (they were marked with timer=0, so re-check) */
        bool chain = true;
        while (chain) {
            chain = false;
            for (int i = 0; i < MAX_BOMBS; i++) {
                if (g->bombs.pool[i].active && g->bombs.pool[i].timer <= 0) {
                    int col = g->bombs.pool[i].col;
                    int row = g->bombs.pool[i].row;
                    bomb_remove(&g->bombs, i);
                    explosion_spawn(&g->explosions, &g->map, &g->bombs, col, row,
                                    g->player.stats.fireRadius,
                                    destroyedBricks, &destroyedCount);
                    audio_play(SFX_EXPLOSION);
                    chain = true;
                }
            }
        }

        /* --- Explosion timers --- */
        explosion_update(&g->explosions);

        /* --- Items: spawn when bricks are destroyed --- */
        for (int i = 0; i < destroyedCount; i++) {
            int col = destroyedBricks[i][0];
            int row = destroyedBricks[i][1];
            /* Reveal the door if it was hidden under this brick */
            if (!g->door.revealed && g->door.col == col && g->door.row == row) {
                g->door.revealed = true; /* visible but still locked */
            }
            /* 35% chance to spawn a power-up from a destroyed brick */
            if (rand() % 100 < 35) {
                item_spawn_random(&g->items, col, row);
            }
        }

        /* --- Enemy update --- */
        enemy_update(&g->enemies, &g->map, &g->bombs, &g->explosions,
                     g->player.base.col, g->player.base.row);

        /* --- Enemy death by explosion --- */
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (g->enemies.pool[i].alive &&
                g->enemies.pool[i].state != ENEMY_STATE_DYING) {
                if (explosion_at(&g->explosions, g->enemies.pool[i].base.col,
                                 g->enemies.pool[i].base.row)) {
                    enemy_kill(&g->enemies, i);
                    g->hud.score += 100;
                }
            }
        }
        enemy_cleanup(&g->enemies);

        /* --- Player death by explosion --- */
        if (!g->player.stats.flamePass &&
            explosion_at(&g->explosions, g->player.base.col, g->player.base.row)) {
            g->player.lives--;
            if (g->player.lives <= 0) {
                g->state = GAME_STATE_GAMEOVER;
                audio_play(SFX_GAMEOVER);
            } else {
                /* NES: death animation, then restart the stage */
                g->player.dead = true;
                g->player.deathTimer = PLAYER_DEATH_FRAMES;
                player_reset_stats(&g->player);
                audio_play(SFX_DEATH);
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (g->enemies.pool[i].alive) enemy_kill(&g->enemies, i);
                }
                return;  /* skip rest of frame */
            }
        }

        /* --- Player death by enemy contact --- */
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (g->enemies.pool[i].alive &&
                g->enemies.pool[i].state != ENEMY_STATE_DYING) {
                if (g->enemies.pool[i].base.col == g->player.base.col &&
                    g->enemies.pool[i].base.row == g->player.base.row) {
                    g->player.lives--;
                    if (g->player.lives <= 0) {
                        g->state = GAME_STATE_GAMEOVER;
                        audio_play(SFX_GAMEOVER);
                    } else {
                        /* NES: death animation, then restart the stage */
                        g->player.dead = true;
                        g->player.deathTimer = PLAYER_DEATH_FRAMES;
                        player_reset_stats(&g->player);
                        audio_play(SFX_DEATH);
                        for (int i = 0; i < MAX_ENEMIES; i++) {
                            if (g->enemies.pool[i].alive) enemy_kill(&g->enemies, i);
                        }
                    }
                    return;  /* skip rest of frame */
                }
            }
        }

        /* --- Item collection --- */
        Item *it = item_at(&g->items, g->player.base.col, g->player.base.row);
        if (it) {
            if (player_apply_item(&g->player, it->type)) {
                item_remove(&g->items, it);
                g->hud.score += 50;
                audio_play(SFX_ITEM);
            }
        }
        item_update(&g->items);

        /* --- Door unlock when all enemies dead --- */
        if (g->enemies.count == 0 && g->door.revealed && !g->door.active) {
            g->door.active = true;
            audio_play(SFX_VICTORY);
        }

        /* --- Door entry (victory) --- */
        if (g->door.active && g->door.revealed &&
            g->player.base.col == g->door.col &&
            g->player.base.row == g->door.row) {
            g->stage++;
            game_reset_stage(g);
            return;
        }

        /* --- HUD timer --- */
        hud_update(&g->hud);
        if (g->hud.timer <= 0) {
            g->player.lives--;
            if (g->player.lives <= 0) {
                g->state = GAME_STATE_GAMEOVER;
            } else {
                game_reset_stage(g);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Draw                                                                */
/* ------------------------------------------------------------------ */
void game_draw(SDL_Renderer *renderer, const Game *g) {
    SDL_SetRenderDrawColor(renderer, 8, 8, 16, 255);
    SDL_RenderClear(renderer);

    if (g->state == GAME_STATE_MENU) {
        SDL_SetRenderDrawColor(renderer, 32, 32, 80, 255);
        SDL_Rect titleRect = {WINDOW_W/2 - 200, 80, 400, 120};
        SDL_RenderFillRect(renderer, &titleRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        hud_draw_text(renderer, WINDOW_W/2 - 90, 110, "BOMBERMAN");
        SDL_Rect button = {WINDOW_W/2 - 100, WINDOW_H/2 - 25, 200, 50};
        SDL_SetRenderDrawColor(renderer, 200, 70, 50, 255);
        SDL_RenderFillRect(renderer, &button);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        hud_draw_text(renderer, WINDOW_W/2 - 45, WINDOW_H/2 - 10, "START");
        hud_draw_text(renderer, WINDOW_W/2 - 120, WINDOW_H/2 + 60, "PRESS ENTER OR CLICK START");
        return;
    }

    /* HUD */
    hud_draw(renderer, &g->hud, &g->player, g->stage);

    /* Map */
    render_map(renderer, &g->map);

    /* Items (under entities) */
    render_items(renderer, &g->items);

    /* Door */
    render_door(renderer, &g->door);

    /* Explosions */
    render_explosions(renderer, &g->explosions);

    /* Enemies */
    render_enemies(renderer, &g->enemies);

    /* Player */
    render_player(renderer, &g->player);

    /* Bombs (drawn on top so they are always visible) */
    render_bombs(renderer, &g->bombs);

    /* Overlays */
    if (g->state == GAME_STATE_VICTORY) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_W, WINDOW_H};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        hud_draw_text(renderer, 180, 200, "VITORIA");
        hud_draw_text(renderer, 120, 260, "PRESS ENTER TO PLAY AGAIN");
    }
    if (g->state == GAME_STATE_GAMEOVER) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_W, WINDOW_H};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        hud_draw_text(renderer, 180, 200, "GAME OVER");
        hud_draw_text(renderer, 120, 260, "PRESS ENTER TO TRY AGAIN");
    }
}