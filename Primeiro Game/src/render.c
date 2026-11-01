#include "render.h"
#include <SDL.h>

/* ------------------------------------------------------------------ */
/* NES-inspired palette (used as fallback)                             */
/* ------------------------------------------------------------------ */
#define C_BLACK   0x00, 0x00, 0x00, 255
#define C_DARK    0x10, 0x10, 0x20, 255
#define C_FLOOR   0x18, 0x18, 0x30, 255
#define C_WALL    0x38, 0x38, 0x50, 255
#define C_WALL_HI 0x50, 0x50, 0x70, 255
#define C_BRICK   0xA0, 0x70, 0x40, 255
#define C_BRICK_HI 0xC0, 0x90, 0x60, 255
#define C_PLAYER  0x00, 0xB0, 0xD0, 255
#define C_PLAYER_D 0x00, 0x70, 0x90, 255
#define C_BOMB    0xC8, 0x20, 0x20, 255
#define C_BOMB_HI 0xFF, 0xB0, 0x40, 255
#define C_FIRE    0xFF, 0xA0, 0x20, 255
#define C_FIRE_HI 0xFF, 0xE0, 0x60, 255
#define C_ENEMY1  0xE0, 0x50, 0x50, 255
#define C_ENEMY2  0x50, 0xC0, 0x50, 255
#define C_ENEMY3  0xC0, 0x50, 0xE0, 255
#define C_ITEM    0xFF, 0xFF, 0x60, 255
#define C_DOOR    0x80, 0x60, 0x40, 255

/* ------------------------------------------------------------------ */
/* Sprite loading (32-bit BGRA BMP produced by tools/convert script)   */
/* ------------------------------------------------------------------ */
static Sprite sprPlayer;
static Sprite sprBomb;
static Sprite sprEnemy;
static bool spritesLoaded = false;

/* Load a 32-bit BMP into an SDL texture with alpha blending. */
static SDL_Texture *loadBmpTexture(SDL_Renderer *renderer, const char *path,
                                   int *outW, int *outH) {
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) {
        SDL_Log("Failed to load %s: %s", path, SDL_GetError());
        return NULL;
    }

    /* Ensure 32-bit with alpha channel (BGRA) */
    SDL_Surface *converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    if (!converted) {
        SDL_Log("Failed to convert %s: %s", path, SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    /* Enable key-based transparency: pure magenta (255,0,255) is transparent */
    SDL_SetColorKey(converted, SDL_TRUE, SDL_MapRGB(converted->format, 255, 0, 255));

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, converted);
    if (!tex) {
        SDL_Log("Failed to create texture from %s: %s", path, SDL_GetError());
        SDL_FreeSurface(converted);
        SDL_FreeSurface(surface);
        return NULL;
    }

    *outW = converted->w;
    *outH = converted->h;

    SDL_FreeSurface(converted);
    SDL_FreeSurface(surface);
    return tex;
}

bool render_init(SDL_Renderer *renderer) {
    int w, h;
    sprPlayer.texture = loadBmpTexture(renderer, "assets/player.bmp", &w, &h);
    sprPlayer.srcW = w; sprPlayer.srcH = h;
    sprPlayer.dstW = TILE_SIZE; sprPlayer.dstH = TILE_SIZE;

    sprBomb.texture = loadBmpTexture(renderer, "assets/bomb.bmp", &w, &h);
    sprBomb.srcW = w; sprBomb.srcH = h;
    sprBomb.dstW = TILE_SIZE; sprBomb.dstH = TILE_SIZE;

    sprEnemy.texture = loadBmpTexture(renderer, "assets/enemy.bmp", &w, &h);
    sprEnemy.srcW = w; sprEnemy.srcH = h;
    sprEnemy.dstW = TILE_SIZE; sprEnemy.dstH = TILE_SIZE;

    spritesLoaded = (sprPlayer.texture != NULL);
    return spritesLoaded;
}

void render_quit(void) {
    if (sprPlayer.texture) SDL_DestroyTexture(sprPlayer.texture);
    if (sprBomb.texture) SDL_DestroyTexture(sprBomb.texture);
    if (sprEnemy.texture) SDL_DestroyTexture(sprEnemy.texture);
    sprPlayer.texture = NULL;
    sprBomb.texture = NULL;
    sprEnemy.texture = NULL;
    spritesLoaded = false;
}

/* Draw a sprite centered on a tile, scaled to the target draw box. */
static void drawSprite(SDL_Renderer *renderer, const Sprite *spr,
                       float cx, float cy, float w, float h) {
    if (!spr->texture) return;
    SDL_Rect dst = {(int)(cx - w / 2), (int)(cy - h / 2), (int)w, (int)h};
    SDL_RenderCopy(renderer, spr->texture, NULL, &dst);
}

/* ------------------------------------------------------------------ */
/* Map                                                                 */
/* ------------------------------------------------------------------ */
void render_map(SDL_Renderer *renderer, const GameMap *map) {
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            SDL_Rect cell = {COL_TO_PX(col), ROW_TO_PY(row), TILE_SIZE, TILE_SIZE};
            int v = map->cells[row][col];
            if (v == CELL_WALL) {
                SDL_SetRenderDrawColor(renderer, C_WALL);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, C_WALL_HI);
                SDL_Rect top = {cell.x, cell.y, cell.w, 4};
                SDL_RenderFillRect(renderer, &top);
            } else if (v == CELL_BRICK) {
                SDL_SetRenderDrawColor(renderer, C_BRICK);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, C_BRICK_HI);
                SDL_Rect top = {cell.x, cell.y, cell.w, 4};
                SDL_RenderFillRect(renderer, &top);
            } else {
                SDL_SetRenderDrawColor(renderer, C_FLOOR);
                SDL_RenderFillRect(renderer, &cell);
            }
            SDL_SetRenderDrawColor(renderer, C_DARK);
            SDL_RenderDrawRect(renderer, &cell);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Player                                                              */
/* ------------------------------------------------------------------ */
void render_player(SDL_Renderer *renderer, const Player *player) {
    float cx = player->base.x + TILE_SIZE / 2.0f;
    float cy = player->base.y + TILE_SIZE / 2.0f;

    /* Death animation: blink and shrink */
    if (player->dead) {
        int remaining = player->deathTimer;
        if (remaining <= 0) return;
        if ((remaining / 6) % 2 == 0) return;
        int size = (int)(TILE_SIZE * (remaining / (float)PLAYER_DEATH_FRAMES));
        if (size < 4) size = 4;
        SDL_Rect body = {(int)(cx - size / 2), (int)(cy - size / 2), size, size};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &body);
        return;
    }

    if (spritesLoaded) {
        drawSprite(renderer, &sprPlayer, cx, cy, (float)sprPlayer.dstW, (float)sprPlayer.dstH);
    } else {
        SDL_Rect body = {(int)(cx - 12), (int)(cy - 12), 24, 24};
        SDL_SetRenderDrawColor(renderer, C_PLAYER);
        SDL_RenderFillRect(renderer, &body);
        SDL_SetRenderDrawColor(renderer, C_PLAYER_D);
        SDL_RenderDrawRect(renderer, &body);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int ex = (int)cx, ey = (int)cy;
        if (player->base.facing == DIR_RIGHT) ex += 4;
        else if (player->base.facing == DIR_LEFT) ex -= 4;
        else if (player->base.facing == DIR_DOWN) ey += 4;
        else if (player->base.facing == DIR_UP) ey -= 4;
        SDL_Rect eye = {ex - 2, ey - 2, 4, 4};
        SDL_RenderFillRect(renderer, &eye);
    }
}

/* ------------------------------------------------------------------ */
/* Enemies                                                             */
/* ------------------------------------------------------------------ */
void render_enemies(SDL_Renderer *renderer, const EnemySystem *es) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy *e = &es->pool[i];
        if (!e->alive) continue;
        float cx = e->base.x + TILE_SIZE / 2.0f;
        float cy = e->base.y + TILE_SIZE / 2.0f;

        if (e->state == ENEMY_STATE_DYING) {
            int size = (int)(20 * (e->deathTimer / 30.0f));
            SDL_Rect r = {(int)(cx - size / 2), (int)(cy - size / 2), size, size};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &r);
            continue;
        }

        if (spritesLoaded) {
            /* Slight tint per enemy type using alpha blend on a copy is complex;
               simply use the shared sprite for now. */
            drawSprite(renderer, &sprEnemy, cx, cy, (float)sprEnemy.dstW, (float)sprEnemy.dstH);
        } else {
            SDL_Color color;
            switch (e->type) {
                case ENEMY_BALLOON: color = (SDL_Color){0xE0,0x50,0x50,255}; break;
                case ENEMY_ONION:   color = (SDL_Color){0x50,0xC0,0x50,255}; break;
                case ENEMY_DOLL:    color = (SDL_Color){0xC0,0x50,0xE0,255}; break;
                default:            color = (SDL_Color){0xFF,0xFF,0xFF,255}; break;
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_Rect body = {(int)(cx - 11), (int)(cy - 11), 22, 22};
            SDL_RenderFillRect(renderer, &body);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &body);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Bombs                                                               */
/* ------------------------------------------------------------------ */
void render_bombs(SDL_Renderer *renderer, const BombSystem *bs) {
    for (int i = 0; i < MAX_BOMBS; i++) {
        const Bomb *b = &bs->pool[i];
        if (!b->active) continue;
        float cx = COL_TO_PX(b->col) + TILE_SIZE / 2.0f;
        float cy = ROW_TO_PY(b->row) + TILE_SIZE / 2.0f;

        if (spritesLoaded) {
            drawSprite(renderer, &sprBomb, cx, cy, (float)sprBomb.dstW, (float)sprBomb.dstH);
        } else {
            int blink = (b->timer < 30) ? 6 : 12;
            if ((b->phaseTimer / blink) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, C_BOMB);
            } else {
                SDL_SetRenderDrawColor(renderer, C_BOMB_HI);
            }
            SDL_Rect body = {(int)(cx - 10), (int)(cy - 10), 20, 20};
            SDL_RenderFillRect(renderer, &body);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &body);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Explosions                                                          */
/* ------------------------------------------------------------------ */
void render_explosions(SDL_Renderer *renderer, const ExplosionSystem *es) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        const Explosion *e = &es->pool[i];
        if (!e->active) continue;
        float cx = COL_TO_PX(e->col) + TILE_SIZE / 2.0f;
        float cy = ROW_TO_PY(e->row) + TILE_SIZE / 2.0f;

        int size = 18 + (e->timer % 4) * 2;
        SDL_Rect r = {(int)(cx - size / 2), (int)(cy - size / 2), size, size};
        SDL_SetRenderDrawColor(renderer, C_FIRE);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, C_FIRE_HI);
        SDL_RenderDrawRect(renderer, &r);
    }
}

/* ------------------------------------------------------------------ */
/* Items                                                               */
/* ------------------------------------------------------------------ */
void render_items(SDL_Renderer *renderer, const ItemSystem *is) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        const Item *it = &is->pool[i];
        if (!it->active) continue;
        if ((it->blinkTimer / 15) % 2 == 0) continue;

        float cx = COL_TO_PX(it->col) + TILE_SIZE / 2.0f;
        float cy = ROW_TO_PY(it->row) + TILE_SIZE / 2.0f;

        SDL_Color color;
        switch (it->type) {
            case ITEM_EXTRA_BOMB: color = (SDL_Color){0xFF,0x60,0x60,255}; break;
            case ITEM_FIRE_UP:    color = (SDL_Color){0xFF,0xA0,0x20,255}; break;
            case ITEM_SPEED_UP:   color = (SDL_Color){0x60,0xFF,0x60,255}; break;
            case ITEM_WALL_PASS:  color = (SDL_Color){0x60,0x60,0xFF,255}; break;
            case ITEM_BOMB_PASS:  color = (SDL_Color){0xFF,0x60,0xFF,255}; break;
            case ITEM_FLAME_PASS: color = (SDL_Color){0xFF,0xFF,0x60,255}; break;
            case ITEM_DETONATOR:  color = (SDL_Color){0x60,0xFF,0xFF,255}; break;
            default:              color = (SDL_Color){0xFF,0xFF,0xFF,255}; break;
        }
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_Rect r = {(int)(cx - 8), (int)(cy - 8), 16, 16};
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &r);
    }
}

/* ------------------------------------------------------------------ */
/* Door                                                                */
/* ------------------------------------------------------------------ */
void render_door(SDL_Renderer *renderer, const Door *door) {
    if (!door->active || !door->revealed) return;
    float cx = COL_TO_PX(door->col) + TILE_SIZE / 2.0f;
    float cy = ROW_TO_PY(door->row) + TILE_SIZE / 2.0f;
    SDL_SetRenderDrawColor(renderer, C_DOOR);
    SDL_Rect r = {(int)(cx - 12), (int)(cy - 12), 24, 24};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &r);
}