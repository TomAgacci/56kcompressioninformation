/* pulse_layer2.c */

#include "pulse_layer2.h"
#include <stdlib.h>

PulseRLE *compress_pulse_rle(const PulseCode *codes,
                             size_t n,
                             size_t *out_n)
{
    if (n == 0) { *out_n = 0; return NULL; }

    PulseRLE *rle = malloc(sizeof(PulseRLE) * n);
    size_t out = 0, i = 0;

    while (i < n) {
        PulseCode cur = codes[i];
        uint32_t run = 1;
        i++;
        while (i < n &&
               codes[i].dur_code == cur.dur_code &&
               codes[i].pow_code == cur.pow_code) {
            run++;
            i++;
        }
        rle[out].code    = cur;
        rle[out].run_len = run;
        out++;
    }

    *out_n = out;
    return rle;
}

PulseCode *expand_pulse_rle(const PulseRLE *rle,
                            size_t n,
                            size_t *out_n)
{
    size_t total = 0;
    for (size_t i = 0; i < n; i++) total += rle[i].run_len;

    PulseCode *codes = malloc(sizeof(PulseCode) * total);
    size_t pos = 0;

    for (size_t i = 0; i < n; i++) {
        for (uint32_t r = 0; r < rle[i].run_len; r++) {
            codes[pos++] = rle[i].code;
        }
    }

    *out_n = total;
    return codes;
}
