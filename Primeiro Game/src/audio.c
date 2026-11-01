#include "audio.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Procedural sound effects (no external assets)                       */
/* ------------------------------------------------------------------ */
typedef struct {
    float freq;
    float duration;
    float volume;
} SfxPreset;

static const SfxPreset SFX_PRESETS[SFX_COUNT] = {
    [SFX_MOVE]      = { 220.0f, 0.05f, 0.1f },
    [SFX_BOMB]      = { 110.0f, 0.15f, 0.3f },
    [SFX_EXPLOSION] = { 80.0f,  0.4f,  0.5f },
    [SFX_ITEM]      = { 660.0f, 0.1f,  0.2f },
    [SFX_DEATH]     = { 440.0f, 0.5f,  0.4f },
    [SFX_VICTORY]   = { 523.0f, 0.3f,  0.3f },
    [SFX_GAMEOVER]  = { 220.0f, 0.8f,  0.4f },
    [SFX_MENU]      = { 330.0f, 0.05f, 0.1f }
};

/* ------------------------------------------------------------------ */
/* Internal audio state                                                */
/* ------------------------------------------------------------------ */
static SDL_AudioDeviceID g_device = 0;
static SDL_AudioSpec      g_spec;
static int                g_sfxPlaying[SFX_COUNT] = {0};

/* Music state */
static SDL_AudioSpec      g_musicSpec;
static Uint8              *g_musicBuf = NULL;
static Uint32             g_musicLen = 0;
static Uint32             g_musicPos = 0;
static bool               g_musicPlaying = false;

/* ------------------------------------------------------------------ */
/* Audio callback                                                      */
/* ------------------------------------------------------------------ */
static void SDLCALL audioCallback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    SDL_memset(stream, 0, len);

    /* Mix music */
    if (g_musicPlaying && g_musicBuf) {
        int samples = len / (g_musicSpec.channels * sizeof(Sint16));
        Sint16 *out = (Sint16 *)stream;
        for (int i = 0; i < samples; i++) {
            if (g_musicPos >= g_musicLen / (g_musicSpec.channels * sizeof(Sint16))) {
                g_musicPos = 0; /* loop */
            }
            for (int ch = 0; ch < g_musicSpec.channels; ch++) {
                Sint16 sample = ((Sint16 *)g_musicBuf)[g_musicPos * g_musicSpec.channels + ch];
                out[i * g_musicSpec.channels + ch] = sample / 4; /* reduce volume */
            }
            g_musicPos++;
        }
    }

    /* Mix SFX */
    for (int id = 0; id < SFX_COUNT; id++) {
        if (g_sfxPlaying[id] > 0) {
            const SfxPreset *p = &SFX_PRESETS[id];
            int samples = len / sizeof(Sint16);
            Sint16 *out = (Sint16 *)stream;
            float phase = (float)(SDL_GetTicks() % (int)(p->duration * 1000)) / (p->duration * 1000);
            float amp = p->volume * (1.0f - phase);
            Sint16 sample = (Sint16)(amp * 32767.0f * SDL_sinf(phase * 6.2831853f * p->freq));
            for (int i = 0; i < samples; i++) {
                out[i] += sample;
            }
            g_sfxPlaying[id]--;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
bool audio_init(void) {
    SDL_AudioSpec desired = {0};
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 1024;
    desired.callback = audioCallback;
    desired.userdata = NULL;

    g_device = SDL_OpenAudioDevice(NULL, 0, &desired, &g_spec, 0);
    if (!g_device) {
        printf("[AUDIO] Failed to open audio: %s\n", SDL_GetError());
        fflush(stdout);
        return false;
    }

    printf("[AUDIO] Audio device opened OK\n");
    fflush(stdout);
    SDL_PauseAudioDevice(g_device, 0);
    return true;
}

void audio_quit(void) {
    audio_stop_music();
    if (g_device) {
        SDL_CloseAudioDevice(g_device);
        g_device = 0;
    }
}

void audio_play(SfxId id) {
    if (id < 0 || id >= SFX_COUNT) return;
    g_sfxPlaying[id] = (int)(SFX_PRESETS[id].duration * 60);
}

/* Load WAV music from file. Call before audio_play_music. */
static bool loadWav(const char *path) {
    if (g_musicBuf) {
        free(g_musicBuf);
        g_musicBuf = NULL;
    }

    SDL_AudioSpec spec;
    Uint8 *buf;
    Uint32 len;
    if (SDL_LoadWAV(path, &spec, &buf, &len) == NULL) {
        printf("[AUDIO] Failed to load music %s: %s\n", path, SDL_GetError());
        fflush(stdout);
        return false;
    }

    printf("[AUDIO] Loaded %s: %u bytes, %d ch, %d Hz\n", path, len, spec.channels, spec.freq);
    fflush(stdout);
    g_musicSpec = spec;
    g_musicBuf = buf;
    g_musicLen = len;
    g_musicPos = 0;
    return true;
}

void audio_play_music(void) {
    printf("[AUDIO] audio_play_music CALLED, g_musicPlaying=%d\n", g_musicPlaying);
    fflush(stdout);
    if (g_musicPlaying) {
        printf("[AUDIO] early return: already playing\n");
        fflush(stdout);
        return;
    }

    printf("[AUDIO] Trying to load music...\n");
    fflush(stdout);
    if (loadWav("assets/music.wav")) {
        printf("[AUDIO] OK: assets/music.wav\n");
        g_musicPlaying = true;
        g_musicPos = 0;
        fflush(stdout);
    } else if (loadWav("music.wav")) {
        printf("[AUDIO] OK: music.wav\n");
        g_musicPlaying = true;
        g_musicPos = 0;
        fflush(stdout);
    } else {
        printf("[AUDIO] FAIL: no music file found\n");
        fflush(stdout);
    }
}

void audio_stop_music(void) {
    g_musicPlaying = false;
    if (g_musicBuf) {
        SDL_free(g_musicBuf);
        g_musicBuf = NULL;
        g_musicLen = 0;
        g_musicPos = 0;
    }
}