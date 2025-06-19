#ifndef AUDIO_EFFECTS_H
#define AUDIO_EFFECTS_H

#include <sox.h>

// 低域通過フィルタ(LPF)の例
void apply_lpf(sox_sample_t *buffer, size_t length, double cutoff_freq, sox_signalinfo_t *signal_info);

#endif