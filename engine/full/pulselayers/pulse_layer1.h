/* pulse_layer1.h */

#ifndef PULSE_LAYER1_H
#define PULSE_LAYER1_H

#include "pulse_layer0.h"

typedef struct {
    uint16_t dur_code;   /* quantized duration index */
    uint16_t pow_code;   /* quantized power index    */
} PulseCode;

PulseCode *quantize_pulses(const PulseSymbol *pulses,
                           size_t n,
                           size_t *out_n);

PulseSymbol *dequantize_pulses(const PulseCode *codes,
                               size_t n,
                               size_t *out_n);

#endif
