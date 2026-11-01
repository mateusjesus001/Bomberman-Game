#ifndef AUDIO_H
#define AUDIO_H

#include "bomber.h"

/* Sound effects (procedurally generated, no external assets) */
typedef enum {
    SFX_MOVE = 0,
    SFX_BOMB,
    SFX_EXPLOSION,
    SFX_ITEM,
    SFX_DEATH,
    SFX_VICTORY,
    SFX_GAMEOVER,
    SFX_MENU,
    SFX_COUNT
} SfxId;

/* ------------------------------------------------------------------ */
/* Audio API                                                           */
/* ------------------------------------------------------------------ */

/* Initialize the audio subsystem. Returns true on success.           */
bool audio_init(void);

/* Play a sound effect. Safe to call every frame.                     */
void audio_play(SfxId id);

/* Start background music from WAV (loops).                           */
void audio_play_music(void);

/* Stop background music.                                             */
void audio_stop_music(void);

/* Shut down audio.                                                   */
void audio_quit(void);

#endif /* AUDIO_H */