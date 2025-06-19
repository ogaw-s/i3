#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sox.h>
#include <pthread.h>
#include "audio_stream.h"
#include "tcp_stream.h"

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define PRECISION 16

int sock;
sox_format_t *in, *out;

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage:\n  %s server <port>\n  %s client <IP> <port>\n", argv[0], argv[0]);
        return 1;
    }

    int is_server = strcmp(argv[1], "server") == 0;
    int port = atoi(argv[is_server ? 2 : 3]);
    const char *ip = is_server ? NULL : argv[2];

    sock = setup_socket(is_server, ip, port);
    if (sock < 0) return 1;

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


    audio_effect_parameters send_params;
    send_params.apply_lpf = 1; //LPFをかけるかどうか

    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_audio, (void *)&send_params);
    pthread_create(&recv_thread, NULL, recv_audio, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    sox_close(in);
    sox_close(out);
    sox_quit();
    close(sock);
    return 0;
}
