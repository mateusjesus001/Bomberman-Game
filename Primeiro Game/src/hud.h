#ifndef HUD_H
#define HUD_H

#include "bomber.h"
#include "player.h"

/* HUD state */
typedef struct {
    int score;
    int timer;          /* stage countdown (seconds) */
    int frameCounter;   /* frames since stage start  */
} HudData;

/* ------------------------------------------------------------------ */
/* HUD API                                                             */
/* ------------------------------------------------------------------ */

void hud_init(HudData *hud);
void hud_update(HudData *hud);
void hud_draw(SDL_Renderer *renderer, const HudData *hud, const Player *player, int stage);

/* Draw monospaced text (also used by menus). */
void hud_draw_text(SDL_Renderer *renderer, int x, int y, const char *text);

#endif /* HUD_H */