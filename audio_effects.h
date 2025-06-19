#ifndef AUDIO_EFFECTS_H
#define AUDIO_EFFECTS_H

#include <stdint.h>
#include <stdlib.h>
#include <sox.h>

// 32bit sox_sample_t 用ゲート（再生時に使う）
void apply_gate(sox_sample_t *buf, size_t samples, sox_sample_t threshold);

#endif
