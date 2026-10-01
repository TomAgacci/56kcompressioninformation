/* pulse_layer3.c */

#include "pulse_layer3.h"
#include <math.h>
#include <stdlib.h>

static const double META_BASE_DUR   = 1e-6;
static const double META_DUR_SCALE  = 1e-6;
static const double META_BASE_POWER = 0.1;
static const double META_POWER_SCALE =
    0.9 / (double)(DURATION_LEVELS * POWER_LEVELS);

MetaPulse *rle_to_meta_pulses(const PulseRLE *rle,
                              size_t n,
                              size_t *out_n)
{
    MetaPulse *meta = malloc(sizeof(MetaPulse) * n);

    for (size_t i = 0; i < n; i++) {
        uint32_t run = rle[i].run_len;
        uint32_t packed =
            ((uint32_t)rle[i].code.dur_code << 8) |
            ((uint32_t)rle[i].code.pow_code);

        meta[i].duration_s  = META_BASE_DUR   + (double)run   * META_DUR_SCALE;
        meta[i].power_level = META_BASE_POWER + (double)packed * META_POWER_SCALE;
    }

    *out_n = n;
    return meta;
}

PulseRLE *meta_pulses_to_rle(const MetaPulse *meta,
                             size_t n,
                             size_t *out_n)
{
    PulseRLE *rle = malloc(sizeof(PulseRLE) * n);

    for (size_t i = 0; i < n; i++) {
        double d = meta[i].duration_s;
        double p = meta[i].power_level;

        double run_f    = (d - META_BASE_DUR) / META_DUR_SCALE;
        if (run_f < 1.0) run_f = 1.0;
        uint32_t run    = (uint32_t)round(run_f);

        double packed_f = (p - META_BASE_POWER) / META_POWER_SCALE;
        if (packed_f < 0.0) packed_f = 0.0;
        uint32_t packed = (uint32_t)round(packed_f);

        uint16_t d_idx = (packed >> 8) & 0x3FF;
        uint16_t p_idx = packed & 0xFF;

        rle[i].code.dur_code = d_idx;
        rle[i].code.pow_code = p_idx;
        rle[i].run_len       = run;
    }

    *out_n = n;
    return rle;
}
