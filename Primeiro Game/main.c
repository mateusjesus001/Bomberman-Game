#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include "src/game.h"
#include "src/audio.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Bomberman 8-bit",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_W, WINDOW_H, 0);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    srand((unsigned int)time(NULL));
    if (!audio_init()) {
        SDL_Log("Audio init failed");
    }
    render_init(renderer);

    Game game;
    game_init(&game);

    bool running = true;
    Uint32 lastTick = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        int inputX = 0, inputY = 0;
        bool placeBomb = false;
        bool detonate = false;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                SDL_Keycode sym = event.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    running = false;
                }
                if (game.state == GAME_STATE_MENU) {
                    if (sym == SDLK_RETURN) {
                        SDL_Log("Menu -> Start");
                        game_reset_stage(&game);
                        audio_play_music();
                    }
                } else if (game.state == GAME_STATE_PLAYING) {
                    if (sym == SDLK_LEFT || sym == SDLK_a || sym == SDLK_j || sym == SDLK_KP_4) {
                        inputX = -1;
                    } else if (sym == SDLK_RIGHT || sym == SDLK_d || sym == SDLK_l || sym == SDLK_KP_6) {
                        inputX = 1;
                    } else if (sym == SDLK_UP || sym == SDLK_w || sym == SDLK_i || sym == SDLK_KP_8) {
                        inputY = -1;
                    } else if (sym == SDLK_DOWN || sym == SDLK_s || sym == SDLK_k || sym == SDLK_KP_2) {
                        inputY = 1;
                    } else if (sym == SDLK_SPACE || sym == SDLK_z || sym == SDLK_x || sym == SDLK_c ||
                               sym == SDLK_LCTRL || sym == SDLK_RCTRL || sym == SDLK_KP_0) {
                        placeBomb = true;
                    } else if (sym == SDLK_RETURN || sym == SDLK_e) {
                        detonate = true;
                    }
                } else if (game.state == GAME_STATE_VICTORY || game.state == GAME_STATE_GAMEOVER) {
                    if (sym == SDLK_RETURN) {
                        game.stage = 0;
                        game_reset_stage(&game);
                    }
                }
            }
            if (game.state == GAME_STATE_MENU && event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_Rect button = {WINDOW_W/2 - 100, WINDOW_H/2 - 25, 200, 50};
                if (mx >= button.x && mx <= button.x + button.w &&
                    my >= button.y && my <= button.y + button.h) {
                    game_reset_stage(&game);
                }
            }
        }

        /* Continuous input: read keyboard state for held keys */
        if (game.state == GAME_STATE_PLAYING) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            inputX = 0;
            inputY = 0;
            if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_J]) inputX = -1;
            else if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_L]) inputX = 1;
            if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_I]) inputY = -1;
            else if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_K]) inputY = 1;
        }

        Uint32 current = SDL_GetTicks();
        if (current - lastTick < 16) {
            SDL_Delay(1);
            continue;
        }
        lastTick = current;

        game_update(&game, inputX, inputY, placeBomb, detonate);
        game_draw(renderer, &game);

        SDL_RenderPresent(renderer);
    }

    render_quit();
    audio_quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}