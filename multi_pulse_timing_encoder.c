/* multi_pulse_timing_encoder.c
   ------------------------------------------------------------
   Encodes data into multiple precise pulse durations.
   Each pulse carries several bits via quantized timing.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BITS_PER_PULSE  12          // e.g. 4096 distinct durations
#define BASE_DURATION_S 1e-6        // 1 microsecond base
#define STEP_DURATION_S 1e-9        // 1 nanosecond step

/* Map N bits into one pulse duration */
double bits_to_pulse_duration(uint16_t symbol) {
    return BASE_DURATION_S + (double)symbol * STEP_DURATION_S;
}

/* Encode a byte stream into pulse durations */
size_t encode_bytes_to_pulses(const uint8_t *data, size_t len, double **out) {
    size_t num_symbols = (len * 8 + BITS_PER_PULSE - 1) / BITS_PER_PULSE;
    double *pulses = malloc(sizeof(double) * num_symbols);

    uint64_t bitbuf = 0;
    int bits_in_buf = 0;
    size_t p = 0;

    for (size_t i = 0; i < len; i++) {
        bitbuf = (bitbuf << 8) | data[i];
        bits_in_buf += 8;

        while (bits_in_buf >= BITS_PER_PULSE) {
            uint16_t symbol = (bitbuf >> (bits_in_buf - BITS_PER_PULSE)) & ((1u << BITS_PER_PULSE) - 1);
            bits_in_buf -= BITS_PER_PULSE;
            pulses[p++] = bits_to_pulse_duration(symbol);
        }
    }

    *out = pulses;
    return p;
}

/* Decode pulse durations back into bytes (ideal, no noise) */
uint8_t *decode_pulses_to_bytes(const double *pulses, size_t num_pulses, size_t out_len) {
    uint8_t *out = malloc(out_len);
    uint64_t bitbuf = 0;
    int bits_in_buf = 0;
    size_t o = 0;

    for (size_t i = 0; i < num_pulses; i++) {
        double t = pulses[i];
        uint16_t symbol = (uint16_t)((t - BASE_DURATION_S) / STEP_DURATION_S + 0.5);
        bitbuf = (bitbuf << BITS_PER_PULSE) | symbol;
        bits_in_buf += BITS_PER_PULSE;

        while (bits_in_buf >= 8 && o < out_len) {
            uint8_t b = (bitbuf >> (bits_in_buf - 8)) & 0xFF;
            bits_in_buf -= 8;
            out[o++] = b;
        }
    }

    return out;
}
