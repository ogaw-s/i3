#ifndef CHAT_STREAM_H
#define CHAT_STREAM_H

#include <pthread.h>

void *send_chat(void *arg);
void *recv_chat(void *arg);

#endif
