/* compression_and_timing.c
   ------------------------------------------------------------
   Pulse-ohm / PCF-style compression
   File-size → power/timing gradient
   Patterns → timing/pulse symbols
   Multi-pulse timing encoder
   Satellite-safe timing quantizer
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t  value;
    uint32_t count;
    uint32_t pulse_code;
} PulsePattern;

typedef struct {
    double  duration_s;
    double  power_level;
} PulseSymbol;

/* --- FILE → pulse-ohm / PCF compression (simple RLE) --- */
size_t compress_to_pulse_patterns(const uint8_t *data, size_t len,
                                  PulsePattern **out)
{
    PulsePattern *patterns = malloc(sizeof(PulsePattern) * len);
    size_t n = 0;

    if (len == 0) {
        *out = NULL;
        return 0;
    }

    uint8_t  current = data[0];
    uint32_t count   = 1;

    for (size_t i = 1; i < len; i++) {
        if (data[i] == current) {
            count++;
        } else {
            patterns[n].value      = current;
            patterns[n].count      = count;
            patterns[n].pulse_code = 0x56000000 | (n & 0xFFFF);
            n++;

            current = data[i];
            count   = 1;
        }
    }

    patterns[n].value      = current;
    patterns[n].count      = count;
    patterns[n].pulse_code = 0x56000000 | (n & 0xFFFF);
    n++;

    *out = patterns;
    return n;
}

/* --- file size → power/timing gradient --- */
#define MIN_POWER_LEVEL   0.1
#define MAX_POWER_LEVEL   1.0
#define MIN_DURATION_S    1e-6
#define MAX_DURATION_S    1e-2

double size_to_gradient(size_t size_bytes, size_t min_bytes, size_t max_bytes) {
    if (size_bytes <= min_bytes) return 0.0;
    if (size_bytes >= max_bytes) return 1.0;
    return (double)(size_bytes - min_bytes) / (double)(max_bytes - min_bytes);
}

double gradient_to_power(double g) {
    return MIN_POWER_LEVEL + g * (MAX_POWER_LEVEL - MIN_POWER_LEVEL);
}

double gradient_to_duration(double g) {
    return MIN_DURATION_S + g * (MAX_DURATION_S - MIN_DURATION_S);
}

/* --- patterns → timing/pulse symbols --- */
double count_to_gradient(uint32_t count, uint32_t max_count) {
    if (count >= max_count) return 1.0;
    return (double)count / (double)max_count;
}

PulseSymbol *patterns_to_pulses(const PulsePattern *patterns, size_t n,
                                uint32_t max_count, double base_power,
                                double base_duration, size_t *out_n)
{
    PulseSymbol *pulses = malloc(sizeof(PulseSymbol) * n);

    for (size_t i = 0; i < n; i++) {
        double g = count_to_gradient(patterns[i].count, max_count);
        pulses[i].power_level = base_power * (0.5 + g);
        pulses[i].duration_s  = base_duration * (0.5 + g);
    }

    *out_n = n;
    return pulses;
}

/* --- multi-pulse timing encoder (bits → durations) --- */
#define BITS_PER_PULSE  12
#define BASE_STEP_S     1e-9

double bits_to_pulse_duration(uint16_t symbol) {
    return MIN_DURATION_S + (double)symbol * BASE_STEP_S;
}

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
            uint16_t symbol = (bitbuf >> (bits_in_buf - BITS_PER_PULSE)) &
                              ((1u << BITS_PER_PULSE) - 1);
            bits_in_buf -= BITS_PER_PULSE;
            pulses[p++] = bits_to_pulse_duration(symbol);
        }
    }

    *out = pulses;
    return p;
}

/* --- satellite-safe timing quantizer --- */
#define SAFE_BITS_PER_PULSE   8
#define SAFE_STEP_S           5e-9
#define SAFE_TOLERANCE_S      3e-9

int quantize_duration(double measured, uint16_t *symbol_out) {
    double idx = measured / SAFE_STEP_S;
    uint16_t s = (uint16_t)(idx + 0.5);

    double ideal = (double)s * SAFE_STEP_S;
    double err = measured - ideal;

    if (err > SAFE_TOLERANCE_S || err < -SAFE_TOLERANCE_S) {
        return 0;
    }

    *symbol_out = s & ((1u << SAFE_BITS_PER_PULSE) - 1);
    return 1;
}
