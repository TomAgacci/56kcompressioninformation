#include "modem_and_morse.h"
#include <stdio.h>

void send_pulses_over_56k_phonic(const PulseSymbol *pulses,
                                 size_t n)
{
    printf("[56k] Sending %zu pulse symbols over phonic channel...\n", n);
    (void)pulses;
}

PulseSymbol *recv_pulses_over_56k_phonic(size_t *out_n)
{
    size_t n = 16;
    PulseSymbol *p = malloc(sizeof(PulseSymbol) * n);
    for (size_t i = 0; i < n; i++) {
        p[i].duration_s  = (i % 2) ? MIN_DURATION_S : MAX_DURATION_S;
        p[i].power_level = (i % 2) ? MIN_POWER_LEVEL : MAX_POWER_LEVEL;
    }
    *out_n = n;
    printf("[56k] Received %zu pulse symbols from phonic channel.\n", n);
    return p;
}

MorseSymbol *pulses_to_morse(const PulseSymbol *pulses,
                             size_t n,
                             size_t *out_n)
{
    MorseSymbol *m = malloc(sizeof(MorseSymbol) * n);
    for (size_t i = 0; i < n; i++) {
        m[i].symbol = (pulses[i].duration_s <
                       (MIN_DURATION_S + MAX_DURATION_S) / 2.0) ? '.' : '-';
    }
    *out_n = n;
    printf("[Morse] Encoded %zu pulses to Morse.\n", n);
    return m;
}

PulseSymbol *morse_to_pulses(const MorseSymbol *m,
                             size_t n,
                             size_t *out_n)
{
    PulseSymbol *p = malloc(sizeof(PulseSymbol) * n);
    for (size_t i = 0; i < n; i++) {
        p[i].duration_s  = (m[i].symbol == '.') ? MIN_DURATION_S : MAX_DURATION_S;
        p[i].power_level = (m[i].symbol == '.') ? MIN_POWER_LEVEL : MAX_POWER_LEVEL;
    }
    *out_n = n;
    printf("[Morse] Decoded %zu Morse symbols to pulses.\n", n);
    return p;
}
