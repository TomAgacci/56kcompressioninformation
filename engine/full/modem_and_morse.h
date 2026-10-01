#ifndef MODEM_AND_MORSE_H
#define MODEM_AND_MORSE_H

#include <stdint.h>
#include <stdlib.h>
#include "compression_and_timing.h"

typedef struct {
    char symbol;   /* '.' or '-' */
} MorseSymbol;

void send_pulses_over_56k_phonic(const PulseSymbol *pulses,
                                 size_t n);

PulseSymbol *recv_pulses_over_56k_phonic(size_t *out_n);

MorseSymbol *pulses_to_morse(const PulseSymbol *pulses,
                             size_t n,
                             size_t *out_n);

PulseSymbol *morse_to_pulses(const MorseSymbol *m,
                             size_t n,
                             size_t *out_n);

#endif
