#include "audio_effects.h"
#include "audio_stream.h"

#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#define BUFFER_SAMPLE_SIZE 4096

extern int sock1;          /* 送信用ソケット  : send_audio() と共有 */
extern int muted;          /* /m で切り替えているフラグ            */

void apply_gate(sox_sample_t *buf, size_t samples, sox_sample_t threshold) {
    int is_loud = 0;
    for (size_t i = 0; i < samples; i++) {
        if (abs(buf[i]) >= threshold) {
            is_loud = 1;
            break;
        }
    }

    if (!is_loud) {
        for (size_t i = 0; i < samples; i++) {
            buf[i] = 0;
        }
    }
}

void send_audio_file(const char *filename)
{
    // ファイルを開く
    sox_format_t *src = sox_open_read(filename, NULL, NULL, NULL);
    if (!src) {
        fprintf(stderr, "send_audio_file: \"%s\" を開けません\n", filename);
        return;
    }

    //バッファ確保
    sox_sample_t *read_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    int16_t      *send_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    if (!read_buf || !send_buf) {
        perror("send_audio_file: malloc");
        goto cleanup;
    }

    const double rate = src->signal.rate;
    const suseconds_t frame_usec = (suseconds_t)(1e6 * BUFFER_SAMPLE_SIZE / rate);

    /* 3. 読み出して送信 */
    size_t samples;
    while ((samples = sox_read(src, read_buf, BUFFER_SAMPLE_SIZE)) > 0) {
        //ミュート中なら送らない
        if (muted) {
            usleep(frame_usec);
            continue;
        }

        // 32 bit → 16 bit 変換 (上位 16bit をそのまま)
        for (size_t i = 0; i < samples; ++i)
            send_buf[i] = (int16_t)(read_buf[i] >> 16);

        // 送る
        ssize_t bytes = samples * sizeof(int16_t);
        if (send(sock1, send_buf, bytes, 0) <= 0) {
            perror("send_audio_file: send");
            break;
        }
    }

cleanup:
    if (src)      sox_close(src);
    free(read_buf);
    free(send_buf);
}
