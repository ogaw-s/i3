#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sox.h>

#define BUFFER_SAMPLE_SIZE 2048  // ここを「サンプル数」として統一
#define SAMPLE_RATE 44100
#define CHANNELS 1
#define PRECISION 16

int sock;
sox_format_t *in, *out;

void *send_audio(void *arg) {
    sox_sample_t *read_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    int16_t *send_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    size_t samples;

    if (!read_buf || !send_buf) {
        perror("malloc");
        exit(1);
    }

    while ((samples = sox_read(in, read_buf, BUFFER_SAMPLE_SIZE)) > 0) {
        for (size_t i = 0; i < samples; ++i)
            send_buf[i] = read_buf[i] >> 16;
        ssize_t bytes = samples * sizeof(int16_t);
        if (send(sock, send_buf, bytes, 0) <= 0)
            break;
    }

    free(read_buf);
    free(send_buf);
    return NULL;
}

void *recv_audio(void *arg) {
    int16_t *recv_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    sox_sample_t *sox_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    ssize_t n;

    if (!recv_buf || !sox_buf) {
        perror("malloc");
        exit(1);
    }

    while (1) {
        n = recv(sock, recv_buf, BUFFER_SAMPLE_SIZE * sizeof(int16_t), 0);
        if (n <= 0) break;

        size_t samples = n / sizeof(int16_t);
        if (samples == 0) continue;

        for (size_t i = 0; i < samples; ++i)
            sox_buf[i] = recv_buf[i] << 16;

        if (sox_write(out, sox_buf, samples) != samples) {
            fprintf(stderr, "sox_write failed\n");
            break;
        }
    }

    free(recv_buf);
    free(sox_buf);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage:\n  %s server <port>\n  %s client <IP> <port>\n", argv[0], argv[0]);
        return 1;
    }

    int is_server = strcmp(argv[1], "server") == 0;
    int port = atoi(argv[is_server ? 2 : 3]);
    const char *ip = is_server ? NULL : argv[2];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    if (is_server) {
        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        listen(sock, 1);

        printf("Waiting for connection on port %d...\n", port);
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &len);
        if (client_sock < 0) { perror("accept"); close(sock); return 1; }
        close(sock);
        sock = client_sock;
    } else {
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            close(sock);
            return 1;
        }
    }

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

    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_audio, NULL);
    pthread_create(&recv_thread, NULL, recv_audio, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    sox_close(in);
    sox_close(out);
    sox_quit();
    close(sock);
    return 0;
}
