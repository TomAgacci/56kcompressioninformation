/* pulse_layer1.c */

#include "pulse_layer1.h"
#include <math.h>

#define DURATION_MIN_S   1e-6
#define DURATION_MAX_S   1e-2
#define POWER_MIN        0.1
#define POWER_MAX        1.0
#define DURATION_LEVELS  1024
#define POWER_LEVELS     256

static double clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

PulseCode *quantize_pulses(const PulseSymbol *pulses,
                           size_t n,
                           size_t *out_n)
{
    PulseCode *codes = malloc(sizeof(PulseCode) * n);
    double dr = DURATION_MAX_S - DURATION_MIN_S;
    double pr = POWER_MAX      - POWER_MIN;

    for (size_t i = 0; i < n; i++) {
        double d = clamp(pulses[i].duration_s, DURATION_MIN_S, DURATION_MAX_S);
        double p = clamp(pulses[i].power_level, POWER_MIN, POWER_MAX);

        double dn = (d - DURATION_MIN_S) / dr;
        double pn = (p - POWER_MIN)      / pr;

        codes[i].dur_code = (uint16_t)round(dn * (DURATION_LEVELS - 1));
        codes[i].pow_code = (uint16_t)round(pn * (POWER_LEVELS   - 1));
    }

    *out_n = n;
    return codes;
}

PulseSymbol *dequantize_pulses(const PulseCode *codes,
                               size_t n,
                               size_t *out_n)
{
    PulseSymbol *pulses = malloc(sizeof(PulseSymbol) * n);
    double dr = DURATION_MAX_S - DURATION_MIN_S;
    double pr = POWER_MAX      - POWER_MIN;

    for (size_t i = 0; i < n; i++) {
        double dn = (double)codes[i].dur_code / (double)(DURATION_LEVELS - 1);
        double pn = (double)codes[i].pow_code / (double)(POWER_LEVELS   - 1);

        pulses[i].duration_s  = DURATION_MIN_S + dn * dr;
        pulses[i].power_level = POWER_MIN      + pn * pr;
    }

    *out_n = n;
    return pulses;
}
