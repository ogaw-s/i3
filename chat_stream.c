#include "chat_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>
#include "audio_stream.h"
#include "chat_stream.h"

#define CHAT_BUF 2048

extern int sock2;
extern int muted;
extern sox_format_t *in, *out;

void *send_chat(void *arg)
{
    char msg[CHAT_BUF];

    while (fgets(msg, sizeof(msg), stdin)) {
        msg[strcspn(msg, "\n")] = '\0'; //改行をとる

        char buf[CHAT_BUF + 6];
        if (strcmp(msg, "/m") == 0) {
            muted = !muted;
            snprintf(buf, sizeof(buf), "CHAT:%s\n",
                     muted ? "相手がミュートしました"
                           : "相手がミュートを解除しました");
        }else if (strcmp(msg, "/yeah") == 0){
            send_audio_file("audiofile/yeah.raw");
        } else {
            snprintf(buf, sizeof(buf), "CHAT:%s\n", msg);
        }
        ssize_t sent = write(sock2, buf, strlen(buf));

        if (sent < 0) {
            perror("send_chat: write");
            break;
        }
    }
    return NULL;
}


void *recv_chat(void *arg) {
    char buf[CHAT_BUF];
    ssize_t n;
    while ((n = read(sock2, buf, sizeof(buf))) > 0) {
        if (n >= 5 && strncmp(buf, "CHAT:", 5) == 0) {
            fwrite("[CHAT] ", 1, 7, stderr);
            fwrite(buf + 5, 1, n - 5, stderr);
            continue;
        }

        if (fwrite(buf, 1, n, stdout) < n) {
            perror("recv_chat: fwrite");
            break;
        }
        fflush(stdout);
    }

    if (n < 0) {
        perror("recv_chat: read");
    }
    return NULL;
}
