// audio_effects.c
#include "audio_effects.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

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

int if_muted(sox_sample_t *buf, size_t samples, sox_sample_t threshold) {
    for (size_t i = 0; i < samples; i++) {
        if (abs(buf[i]) >= threshold) {
            return 0;  // 音あり → ミュートしない
        }
    }
    return 1;  // 音なし → ミュート（ゲートON）
}
