/* pulse_layer2.h */

#ifndef PULSE_LAYER2_H
#define PULSE_LAYER2_H

#include "pulse_layer1.h"

typedef struct {
    PulseCode code;
    uint32_t  run_len;
} PulseRLE;

PulseRLE *compress_pulse_rle(const PulseCode *codes,
                             size_t n,
                             size_t *out_n);

PulseCode *expand_pulse_rle(const PulseRLE *rle,
                            size_t n,
                            size_t *out_n);

#endif
