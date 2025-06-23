#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sox.h>
#include <pthread.h>
#include "audio_stream.h"
#include "tcp_stream.h"
#include "chat_stream.h"
#include "gtk_GUI.h"

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define PRECISION 16

int sock1; // 音声
int sock2; // チャット
int muted = 0; // ミュートするかどうかの変数
sox_format_t *in, *out;

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage:\n  %s server <port>\n  %s client <IP> <port>\n", argv[0], argv[0]);
        return 1;
    }

    int is_server = strcmp(argv[1], "server") == 0;
    int port = atoi(argv[is_server ? 2 : 3]);
    const char *ip = is_server ? NULL : argv[2];

    sock1 = setup_socket(is_server, ip, port);
    if (sock1 < 0) return 1;
    // 少し待ってから次の接続を試みる
    usleep(200000); // 0.2秒待つ（調整可能）
    sock2 = setup_socket(is_server, ip, port + 1);
    if (sock2 < 0) return 1;

    sox_init();

    sox_signalinfo_t signal = {
        .rate = SAMPLE_RATE,
        .channels = CHANNELS,
        .precision = PRECISION,
        .length = SOX_UNSPEC,
        .mult = NULL
    };

    in = sox_open_read("default", &signal, NULL, "alsa");
    if (!in) { fprintf(stderr, "Failed to open mic input\n"); return 1; }

    out = sox_open_write("default", &signal, NULL, "alsa", NULL, NULL);
    if (!out) { fprintf(stderr, "Failed to open speaker output\n"); return 1; }

    pthread_t send_audio_thread, recv_audio_thread, send_chat_thread, recv_chat_thread;
    pthread_create(&send_audio_thread, NULL, send_audio, NULL);
    pthread_create(&recv_audio_thread, NULL, recv_audio, NULL);
    pthread_create(&send_chat_thread, NULL, send_chat, NULL);
    pthread_create(&recv_chat_thread, NULL, recv_chat, NULL);

    pthread_join(send_audio_thread, NULL);
    pthread_join(recv_audio_thread, NULL);
    pthread_join(send_chat_thread, NULL);
    pthread_join(recv_chat_thread, NULL);

    sox_close(in);
    sox_close(out);
    sox_quit();
    close(sock1);
    close(sock2);
    return 0;
}
