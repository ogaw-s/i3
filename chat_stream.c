#include "chat_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>
#define CHAT_BUF 2048

extern int sock2;
extern int muted_mine;
extern int muted_partner;
extern char message_mine[CHAT_BUF];
extern char message_partner[CHAT_BUF];
extern int send_flag;
extern int recv_flag;
extern sox_format_t *in, *out;

void *send_chat(void *arg) {
    while (1) {
        if (send_flag){
            ssize_t sent = write(sock2, message_mine, CHAT_BUF);
            if (sent < 0) {
                perror("send_chat: write");
                break;
            }
            memset(message_mine, 0, CHAT_BUF);
            send_flag = 0;
        }
    }
    return NULL;
}

void *recv_chat(void *arg) {
    ssize_t n;

    while ((n = read(sock2, message_partner, CHAT_BUF)) > 0) {
        if(n >= 5 && strncmp(message_partner, "/m_ON", 5) == 0) {
            muted_partner = 1;
            continue;
        } else if(n >= 6 && strncmp(message_partner, "/m_OFF", 6) == 0){
            muted_partner = 0;
            continue;
        }
        recv_flag = n;
    }

    if (n < 0) {
        perror("recv_chat: read");
    }
    return NULL;
}
