#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

void init_audio();
void shutdown_audio();
void play_music(const char* filename);
void play_sound_effect(const char* sound_name); // e.g., "jump", "crash"

#endif // AUDIO_H