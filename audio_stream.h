#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <pthread.h>

void *send_audio(void *arg);
void *recv_audio(void *arg);

typedef struct audio_effect_parameters {
    int apply_lpf;  // 1ならLPFをかける、0ならかけない
} audio_effect_parameters;

#endif
