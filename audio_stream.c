#include "audio_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SAMPLE_SIZE 2048

extern int sock;
extern sox_format_t *in, *out;

void *send_audio(void *arg) {
    sox_sample_t *read_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    int16_t *send_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    size_t samples;

    if (!read_buf || !send_buf) {
        perror("malloc");
        exit(1);
    }

    sox_sample_t sample;
    while ((samples = sox_read(in, read_buf, BUFFER_SAMPLE_SIZE)) > 0) {
        for (size_t i = 0; i < samples; ++i) {
            sample = read_buf[i] >> 16;
            if (abs(sample) < 5000) {
                send_buf[i] = 0;
            }else {
                send_buf[i] = sample;
            }
        }
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
    
    int recv_buf_sample_num = 0;
    while (1) {
        n = recv(sock, recv_buf, BUFFER_SAMPLE_SIZE * sizeof(int16_t), 0);
        if (n <= 0) break;

        size_t samples = n / sizeof(int16_t);
        recv_buf_sample_num += samples;
        if (samples == 0) continue;

        if (recv_buf_sample_num >= 8800 * 5){
            for (size_t i = 0; i < samples; ++i)
            sox_buf[i] = recv_buf[i] << 16;
        }
        
        if (sox_write(out, sox_buf, samples) != samples) {
            fprintf(stderr, "sox_write failed\n");
            break;
        }
    }

    free(recv_buf);
    free(sox_buf);
    return NULL;
}
