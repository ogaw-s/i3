#include "chat_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>

#define CHAT_BUF 2048

extern int sock1;
extern muted;
extern sox_format_t *in, *out;

void *send_chat(void *arg) {
    char msg[CHAT_BUF];
    while (fgets(msg, sizeof(msg), stdin)) {
        if (strncmp(msg, "/m", 2) == 0) {
            muted = !muted;
            fprintf(stderr, "[ミュート %s]\n", muted ? "ON" : "OFF");
            continue;
        }

        char buf[CHAT_BUF + 6];
        snprintf(buf, sizeof(buf), "CHAT:%s", msg);

        ssize_t sent = write(sock1, buf, strlen(buf));
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
    while ((n = read(sock1, buf, sizeof(buf))) > 0) {
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
