#include "audio_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>
#include "audio_effects.h"
#define BUFFER_SAMPLE_SIZE 2048

extern int sock;
extern sox_format_t *in, *out;

#include "audio_effects.h"

typedef struct {
    int apply_lpf;  // 1ならLPFをかける、0ならかけない
} audio_send_params_t;

void *send_audio(void *arg) {
    audio_send_params_t *params = (audio_send_params_t *)arg;

    sox_sample_t *read_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    int16_t *send_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    size_t samples;

    if (!read_buf || !send_buf) {
        perror("malloc");
        exit(1);
    }

    while ((samples = sox_read(in, read_buf, BUFFER_SAMPLE_SIZE)) > 0) {
        if (params && params->apply_lpf) {
            apply_lpf(read_buf, samples, 3000.0, &in->signal); // 3kHzカットオフ例
        }

        for (size_t i = 0; i < samples; ++i) {
            sox_sample_t sample = read_buf[i] >> 16;
            send_buf[i] = (abs(sample) < 5000) ? 0 : sample;
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
    // 音声の受信
    int16_t *recv_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t)); //16bitの受信用バッファ
    sox_sample_t *sox_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t)); //再生は32bit
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
        // sox_writeで再生
        if (sox_write(out, sox_buf, samples) != samples) { 
            fprintf(stderr, "sox_write failed\n");
            break;
        }
    }

    free(recv_buf);
    free(sox_buf);
    return NULL;
}
