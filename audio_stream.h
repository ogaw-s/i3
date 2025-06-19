#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <pthread.h>

void *send_audio(void *arg);
void *recv_audio(void *arg);

#endif
