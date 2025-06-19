#include "audio_effects.h"
#include <math.h>

void apply_lpf(sox_sample_t *buffer, size_t length, double cutoff_freq, sox_signalinfo_t *signal_info) {
    // サンプリング周波数取得
    double fs = signal_info->rate;

    // 正規化カットオフ周波数
    double dt = 1.0 / fs;
    double RC = 1.0 / (2 * M_PI * cutoff_freq);
    double alpha = dt / (RC + dt);

    sox_sample_t prev = 0;
    for (size_t i = 0; i < length; i++) {
        // シンプルな1次IIR LPF (y[n] = y[n-1] + α(x[n] - y[n-1]))
        buffer[i] = prev + alpha * (buffer[i] - prev);
        prev = buffer[i];
    }
}
