#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sox.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h> // エラー情報のためにerrnoを含める

#include "audio_effects.h"
#include "audio_stream.h"

#define BUFFER_SAMPLE_SIZE 2048

extern int sock1;
extern sox_format_t *in, *out;
extern int muted;

void *send_audio(void *arg) {
    // 音声の送信
    sox_sample_t *read_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t)); // 32bit libsox sample buffer
    int16_t *send_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));          // 16bit buffer to send
    size_t samples;

    if (!read_buf || !send_buf) {
        perror("malloc failed in send_audio");
        exit(1);
    }

    sox_sample_t sample;
    while ((samples = sox_read(in, read_buf, BUFFER_SAMPLE_SIZE)) > 0) {
        if (!muted) {
            for (size_t i = 0; i < samples; ++i) {
                // Soxの32ビットサンプルを16ビットに変換
                // 注意: sox_sample_t は SOX_SAMPLE_MAX を基準とした正規化された値です。
                // 16ビットへの変換は、必要に応じて飽和処理を考慮する必要がありますが、
                // 通常は右シフトで十分です。
                sample = read_buf[i] >> 16;
                send_buf[i] = (int16_t)sample;
            }

            ssize_t bytes_to_send = samples * sizeof(int16_t);
            ssize_t total_sent = 0;

            // 送信完了までループ
            while (total_sent < bytes_to_send) {
                ssize_t sent = send(sock1, (char *)send_buf + total_sent, bytes_to_send - total_sent, 0);
                if (sent < 0) {
                    perror("send failed in send_audio");
                    // エラー発生時は送信ループを抜ける
                    break;
                }
                if (sent == 0) {
                    fprintf(stderr, "Connection closed by peer during send_audio.\n");
                    // 接続が閉じられた場合は送信ループを抜ける
                    break;
                }
                total_sent += sent;
            }
            
            // チャンク全体の送信が完了しなかった場合は、メインの音声読み込みループも抜ける
            if (total_sent < bytes_to_send) {
                break;
            }
        }
    }

    free(read_buf);
    free(send_buf);
    return NULL;
}

void *recv_audio(void *arg) {
    int16_t *recv_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(int16_t));
    sox_sample_t *sox_buf = malloc(BUFFER_SAMPLE_SIZE * sizeof(sox_sample_t));
    
    if (!recv_buf || !sox_buf) {
        perror("malloc failed in recv_audio");
        exit(1);
    }

    while (1) {
        ssize_t bytes_to_read = BUFFER_SAMPLE_SIZE * sizeof(int16_t);
        ssize_t total_received = 0;

        // 受信完了までループ
        while (total_received < bytes_to_read) {
            ssize_t received = recv(sock1, (char *)recv_buf + total_received, bytes_to_read - total_received, 0);
            if (received < 0) {
                perror("recv failed in recv_audio");
                // エラー発生時は受信ループを抜ける
                break;
            }
            if (received == 0) {
                fprintf(stderr, "Connection closed by peer during recv_audio.\n");
                // 接続が閉じられた場合は受信ループを抜ける
                goto cleanup; // ループと関数を抜けてリソースを解放する
            }
            total_received += received;
        }

        // チャンク全体の受信が完了しなかった場合は、メインの受信ループも抜ける
        if (total_received < bytes_to_read) {
            break;
        }

        size_t samples = total_received / sizeof(int16_t);
        if (samples == 0) continue; // 受信したデータがゼロの場合は次のループへ

        for (size_t i = 0; i < samples; ++i) {
            // 16ビットサンプルをSoxの32ビットサンプルに変換
            // 元のコードでは '/ 2' がありましたが、おそらく音量を下げる意図だったと思われます。
            // 信号が飽和しないように、必要であれば調整してください。
            sox_buf[i] = (sox_sample_t)recv_buf[i] << 16;
        }

        // 音声ゲートの適用
        apply_gate(sox_buf, samples, 5000 << 16);

        // Soxへの書き込み
        if (sox_write(out, sox_buf, samples) != samples) {
            fprintf(stderr, "sox_write failed in recv_audio.\n");
            break;
        }
    }

cleanup: // goto文のジャンプ先
    free(recv_buf);
    free(sox_buf);
    return NULL;
}