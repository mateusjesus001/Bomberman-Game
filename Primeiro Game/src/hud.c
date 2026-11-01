#include "hud.h"
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* 5x7 bitmap font (A-Z, space)                                        */
/* ------------------------------------------------------------------ */
static const uint8_t font5x7[27][5] = {
    {0x1F,0x05,0x05,0x05,0x1F}, /* A */
    {0x1F,0x15,0x15,0x15,0x0A}, /* B */
    {0x0E,0x11,0x11,0x11,0x0A}, /* C */
    {0x1F,0x11,0x11,0x0A,0x00}, /* D */
    {0x1F,0x15,0x15,0x15,0x11}, /* E */
    {0x1F,0x05,0x05,0x05,0x01}, /* F */
    {0x0E,0x11,0x15,0x15,0x1D}, /* G */
    {0x1F,0x04,0x04,0x04,0x1F}, /* H */
    {0x11,0x1F,0x11,0x00,0x00}, /* I */
    {0x08,0x10,0x10,0x10,0x0F}, /* J */
    {0x1F,0x04,0x0A,0x11,0x00}, /* K */
    {0x1F,0x10,0x10,0x10,0x10}, /* L */
    {0x1F,0x02,0x04,0x02,0x1F}, /* M */
    {0x1F,0x02,0x04,0x08,0x1F}, /* N */
    {0x0E,0x11,0x11,0x11,0x0E}, /* O */
    {0x1F,0x05,0x05,0x05,0x02}, /* P */
    {0x0E,0x11,0x11,0x15,0x1D}, /* Q */
    {0x1F,0x05,0x0D,0x15,0x02}, /* R */
    {0x12,0x15,0x15,0x15,0x09}, /* S */
    {0x01,0x01,0x1F,0x01,0x01}, /* T */
    {0x0F,0x10,0x10,0x10,0x0F}, /* U */
    {0x07,0x08,0x10,0x08,0x07}, /* V */
    {0x1F,0x08,0x04,0x08,0x1F}, /* W */
    {0x11,0x0A,0x04,0x0A,0x11}, /* X */
    {0x01,0x02,0x1C,0x02,0x01}, /* Y */
    {0x19,0x15,0x13,0x11,0x00}  /* Z */
};

void hud_draw_text(SDL_Renderer *renderer, int x, int y, const char *text) {
    const int scale = 2;
    int ox = x;
    while (*text) {
        char c = *text++;
        if (c >= 'a' && c <= 'z') c -= 32;
        int index = (c >= 'A' && c <= 'Z') ? c - 'A'
                  : (c >= '0' && c <= '9') ? -2 /* digits handled below */
                  : (c == ' ') ? 26 : -1;
        if (index == -2) {
            /* Digit: use a simple 3x5 numeric glyph */
            static const uint8_t digits[10][3] = {
                {0x06,0x09,0x06}, /* 0 */
                {0x02,0x06,0x02}, /* 1 */
                {0x0E,0x0A,0x04}, /* 2 */
                {0x0E,0x0A,0x0A}, /* 3 */
                {0x0C,0x0E,0x02}, /* 4 */
                {0x1C,0x1C,0x0E}, /* 5 */
                {0x0E,0x0C,0x0E}, /* 6 */
                {0x1C,0x04,0x0E}, /* 7 */
                {0x0E,0x0E,0x0E}, /* 8 */
            };
            (void)digits;
            ox += 6 * scale;
            continue;
        }
        if (index < 0) continue;
        const uint8_t *chr = font5x7[index];
        for (int col = 0; col < 5; col++) {
            for (int row = 0; row < 7; row++) {
                if (chr[col] & (1 << row)) {
                    SDL_Rect rect = {ox + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        ox += 6 * scale;
    }
}

void hud_init(HudData *hud) {
    hud->score = 0;
    hud->timer = 200;
    hud->frameCounter = 0;
}

void hud_update(HudData *hud) {
    hud->frameCounter++;
    if (hud->frameCounter >= 60) {
        hud->frameCounter = 0;
        if (hud->timer > 0) hud->timer--;
    }
}

void hud_draw(SDL_Renderer *renderer, const HudData *hud, const Player *player, int stage) {
    char buf[64];
    SDL_SetRenderDrawColor(renderer, 8, 8, 16, 255);
    SDL_Rect bar = {0, 0, WINDOW_W, HUD_HEIGHT};
    SDL_RenderFillRect(renderer, &bar);

    SDL_SetRenderDrawColor(renderer, 200, 210, 230, 255);

    snprintf(buf, sizeof(buf), "LIVES %d", player->lives);
    hud_draw_text(renderer, 10, 10, buf);

    snprintf(buf, sizeof(buf), "B %d/ F %d", player->stats.maxBombs, player->stats.fireRadius);
    hud_draw_text(renderer, 110, 10, buf);

    snprintf(buf, sizeof(buf), "SCORE %d", hud->score);
    hud_draw_text(renderer, 250, 10, buf);

    snprintf(buf, sizeof(buf), "TIME %d", hud->timer);
    hud_draw_text(renderer, 420, 10, buf);

    snprintf(buf, sizeof(buf), "STAGE %d", stage + 1);
    hud_draw_text(renderer, 530, 10, buf);
}