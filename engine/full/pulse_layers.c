/* pulse_layers.c
 *
 * Implementation of multi-layer pulse compression.
 */

#include "pulse_layers.h"
#include <math.h>
#include <stdio.h>

/* Tunable quantization parameters */
#define DURATION_MIN_S   1e-6
#define DURATION_MAX_S   1e-2
#define POWER_MIN        0.1
#define POWER_MAX        1.0

#define DURATION_LEVELS  1024   /* 10-bit duration index */
#define POWER_LEVELS     256    /* 8-bit power index     */

/* Clamp helper */
static double clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Layer 1: quantize raw pulses into PulseCode */
PulseCode *quantize_pulses_to_codes(const PulseSymbol *pulses,
                                    size_t n,
                                    size_t *out_n)
{
    PulseCode *codes = (PulseCode *)malloc(sizeof(PulseCode) * n);
    if (!codes) {
        *out_n = 0;
        return NULL;
    }

    double dur_range   = DURATION_MAX_S - DURATION_MIN_S;
    double power_range = POWER_MAX      - POWER_MIN;

    for (size_t i = 0; i < n; i++) {
        double d = clamp(pulses[i].duration_s, DURATION_MIN_S, DURATION_MAX_S);
        double p = clamp(pulses[i].power_level, POWER_MIN, POWER_MAX);

        double d_norm = (d - DURATION_MIN_S) / dur_range;
        double p_norm = (p - POWER_MIN)      / power_range;

        uint16_t d_idx = (uint16_t)round(d_norm * (DURATION_LEVELS - 1));
        uint16_t p_idx = (uint16_t)round(p_norm * (POWER_LEVELS   - 1));

        codes[i].code  = d_idx;
        codes[i].power = p_idx;
    }

    *out_n = n;
    return codes;
}

/* Layer 2: simple RLE over PulseCode stream */
PulseRLE *compress_codes_rle(const PulseCode *codes,
                             size_t n,
                             size_t *out_n)
{
    if (n == 0) {
        *out_n = 0;
        return NULL;
    }

    PulseRLE *rle = (PulseRLE *)malloc(sizeof(PulseRLE) * n);
    if (!rle) {
        *out_n = 0;
        return NULL;
    }

    size_t out = 0;
    size_t i   = 0;

    while (i < n) {
        PulseCode current = codes[i];
        uint32_t run_len  = 1;
        i++;

        while (i < n &&
               codes[i].code  == current.code &&
               codes[i].power == current.power) {
            run_len++;
            i++;
        }

        rle[out].code    = current;
        rle[out].run_len = run_len;
        out++;
    }

    *out_n = out;
    return rle;
}

/* Expand RLE back to PulseCode */
PulseCode *expand_rle_to_codes(const PulseRLE *rle,
                               size_t n,
                               size_t *out_n)
{
    /* First compute total length */
    size_t total = 0;
    for (size_t i = 0; i < n; i++) {
        total += rle[i].run_len;
    }

    PulseCode *codes = (PulseCode *)malloc(sizeof(PulseCode) * total);
    if (!codes) {
        *out_n = 0;
        return NULL;
    }

    size_t pos = 0;
    for (size_t i = 0; i < n; i++) {
        for (uint32_t r = 0; r < rle[i].run_len; r++) {
            codes[pos++] = rle[i].code;
        }
    }

    *out_n = total;
    return codes;
}

/* Layer 3: encode RLE into MetaPulse
 *
 * Idea: each RLE entry becomes a meta-pulse whose:
 *  - duration encodes run_len
 *  - power encodes the PulseCode (duration index + power index)
 */
MetaPulse *encode_rle_to_meta_pulses(const PulseRLE *rle,
                                     size_t n,
                                     size_t *out_n)
{
    MetaPulse *meta = (MetaPulse *)malloc(sizeof(MetaPulse) * n);
    if (!meta) {
        *out_n = 0;
        return NULL;
    }

    /* Choose scaling factors so meta-pulses stay in a reasonable range */
    const double base_duration = 1e-6;   /* 1 microsecond base */
    const double duration_scale = 1e-6;  /* 1 microsecond per run unit */

    const double base_power = 0.1;
    const double power_scale = 0.9 / (double)(DURATION_LEVELS * POWER_LEVELS);

    for (size_t i = 0; i < n; i++) {
        uint32_t run = rle[i].run_len;
        uint32_t packed_code =
            ((uint32_t)rle[i].code.code << 8) |
            ((uint32_t)rle[i].code.power);

        meta[i].duration_s  = base_duration + (double)run * duration_scale;
        meta[i].power_level = base_power + (double)packed_code * power_scale;
    }

    *out_n = n;
    return meta;
}

/* Decode MetaPulse back into RLE
 *
 * Inverse of encode_rle_to_meta_pulses.
 */
PulseRLE *decode_meta_pulses_to_rle(const MetaPulse *meta,
                                    size_t n,
                                    size_t *out_n)
{
    PulseRLE *rle = (PulseRLE *)malloc(sizeof(PulseRLE) * n);
    if (!rle) {
        *out_n = 0;
        return NULL;
    }

    const double base_duration = 1e-6;
    const double duration_scale = 1e-6;

    const double base_power = 0.1;
    const double power_scale = 0.9 / (double)(DURATION_LEVELS * POWER_LEVELS);

    for (size_t i = 0; i < n; i++) {
        double d = meta[i].duration_s;
        double p = meta[i].power_level;

        /* Recover run_len */
        double run_f = (d - base_duration) / duration_scale;
        if (run_f < 1.0) run_f = 1.0;
        uint32_t run = (uint32_t)round(run_f);

        /* Recover packed_code */
        double packed_f = (p - base_power) / power_scale;
        if (packed_f < 0.0) packed_f = 0.0;
        uint32_t packed = (uint32_t)round(packed_f);

        uint16_t d_idx = (packed >> 8) & 0x3FF; /* up to 10 bits */
        uint16_t p_idx = packed & 0xFF;

        rle[i].code.code  = d_idx;
        rle[i].code.power = p_idx;
        rle[i].run_len    = run;
    }

    *out_n = n;
    return rle;
}

/* Utility: reconstruct approximate PulseSymbol from PulseCode */
PulseSymbol *codes_to_pulses(const PulseCode *codes,
                             size_t n,
                             size_t *out_n)
{
    PulseSymbol *pulses = (PulseSymbol *)malloc(sizeof(PulseSymbol) * n);
    if (!pulses) {
        *out_n = 0;
        return NULL;
    }

    double dur_range   = DURATION_MAX_S - DURATION_MIN_S;
    double power_range = POWER_MAX      - POWER_MIN;

    for (size_t i = 0; i < n; i++) {
        double d_norm = (double)codes[i].code / (double)(DURATION_LEVELS - 1);
        double p_norm = (double)codes[i].power / (double)(POWER_LEVELS   - 1);

        pulses[i].duration_s  = DURATION_MIN_S + d_norm * dur_range;
        pulses[i].power_level = POWER_MIN      + p_norm * power_range;
    }

    *out_n = n;
    return pulses;
}
