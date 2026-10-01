/* pulse_layer3.h */

#ifndef PULSE_LAYER3_H
#define PULSE_LAYER3_H

#include "pulse_layer2.h"
#include "pulse_layer0.h"

typedef struct {
    double duration_s;
    double power_level;
} MetaPulse;

MetaPulse *rle_to_meta_pulses(const PulseRLE *rle,
                              size_t n,
                              size_t *out_n);

PulseRLE *meta_pulses_to_rle(const MetaPulse *meta,
                             size_t n,
                             size_t *out_n);

#endif
