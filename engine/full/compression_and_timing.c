#include "compression_and_timing.h"
#include <string.h>
#include <stdio.h>

#define MIN_POWER_LEVEL   0.1
#define MAX_POWER_LEVEL   1.0
#define MIN_DURATION_S    1e-6
#define MAX_DURATION_S    1e-2

#define BITS_PER_PULSE  12
#define BASE_STEP_S     1e-9

#define SAFE_BITS_PER_PULSE   8
#define SAFE_STEP_S           5e-9
#define SAFE_TOLERANCE_S      3e-9

size_t compress_to_pulse_patterns(const uint8_t *data,
                                  size_t len,
                                  PulsePattern **out)
{
    PulsePattern *patterns = malloc(sizeof(PulsePattern) * (len * 2));
    size_t n = 0;
    size_t i = 0;

    while (i < len) {
        size_t run_start = i;
        uint8_t v = data[i];
        size_t run_len = 1;

        while (i + run_len < len && data[i + run_len] == v) {
            run_len++;
        }

        if (run_len >= 4) {
            PulsePattern *p = &patterns[n++];
            p->type      = PATTERN_RUN;
            p->value     = v;
            p->count     = (uint32_t)run_len;
            p->pulse_code= 0x56000000 | (n & 0xFFFF);
            p->literal   = NULL;
            i += run_len;
        } else {
            size_t lit_start = i;
            size_t lit_len   = run_len;
            i += run_len;

            while (i < len) {
                size_t look = i;
                uint8_t lv = data[look];
                size_t lrun = 1;
                while (look + lrun < len && data[look + lrun] == lv) {
                    lrun++;
                }
                if (lrun >= 4) break;
                lit_len += lrun;
                i += lrun;
            }

            PulsePattern *p = &patterns[n++];
            p->type      = PATTERN_LITERAL;
            p->value     = 0;
            p->count     = (uint32_t)lit_len;
            p->pulse_code= 0x56010000 | (n & 0xFFFF);
            p->literal   = malloc(lit_len);
            memcpy(p->literal, data + lit_start, lit_len);
        }
    }

    *out = patterns;
    return n;
}

double size_to_gradient(size_t size_bytes,
                        size_t min_bytes,
                        size_t max_bytes)
{
    if (size_bytes <= min_bytes) return 0.0;
    if (size_bytes >= max_bytes) return 1.0;
    return (double)(size_bytes - min_bytes) /
           (double)(max_bytes - min_bytes);
}

double gradient_to_power(double g) {
    return MIN_POWER_LEVEL + g * (MAX_POWER_LEVEL - MIN_POWER_LEVEL);
}

double gradient_to_duration(double g) {
    return MIN_DURATION_S + g * (MAX_DURATION_S - MIN_DURATION_S);
}

static double count_to_gradient(uint32_t count, uint32_t max_count) {
    if (count >= max_count) return 1.0;
    return (double)count / (double)max_count;
}

PulseSymbol *patterns_to_pulses(const PulsePattern *patterns,
                                size_t n,
                                uint32_t max_count,
                                double base_power,
                                double base_duration,
                                size_t *out_n)
{
    PulseSymbol *pulses = malloc(sizeof(PulseSymbol) * n);

    for (size_t i = 0; i < n; i++) {
        double g = count_to_gradient(patterns[i].count, max_count);
        double type_factor = (patterns[i].type == PATTERN_RUN) ? 1.0 : 0.6;

        pulses[i].power_level = base_power * (0.5 + g) * type_factor;
        pulses[i].duration_s  = base_duration * (0.5 + g) * type_factor;
    }

    *out_n = n;
    return pulses;
}

size_t encode_bytes_to_pulses(const uint8_t *data,
                              size_t len,
                              double **out)
{
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
            pulses[p++] = MIN_DURATION_S + (double)symbol * BASE_STEP_S;
        }
    }

    *out = pulses;
    return p;
}

int quantize_duration(double measured,
                      uint16_t *symbol_out)
{
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
