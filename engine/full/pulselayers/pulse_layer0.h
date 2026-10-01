/* pulse_layer0.h */

#ifndef PULSE_LAYER0_H
#define PULSE_LAYER0_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    double duration_s;   /* raw measured or scheduled duration */
    double power_level;  /* raw amplitude / power */
} PulseSymbol;

/* Generate raw pulses from bytes (your existing mapping can plug here) */
size_t bytes_to_raw_pulses(const uint8_t *data,
                           size_t len,
                           PulseSymbol **out);

/* Measure raw pulses from hardware / modem */
size_t read_raw_pulses_from_device(PulseSymbol **out);

#endif
