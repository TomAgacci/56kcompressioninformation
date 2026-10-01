#ifndef V92_ROUTER_AND_LATENCY_H
#define V92_ROUTER_AND_LATENCY_H

#include <stdint.h>
#include <stdlib.h>
#include "modem_and_morse.h"

typedef struct {
    uint8_t tone_id;
} V92Tone;

typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   read_pos;
    size_t   frontier;
} LatencyBuffer;

V92Tone *morse_to_v92(const MorseSymbol *m,
                      size_t n,
                      size_t *out_n);

MorseSymbol *v92_to_morse(const V92Tone *tones,
                          size_t n,
                          size_t *out_n);

void send_v92_over_router(const V92Tone *tones,
                          size_t n);

V92Tone *recv_v92_from_router(size_t *out_n);

double measure_rtt_s(void);

size_t compute_window_bytes(double bandwidth_bps,
                            double rtt_s);

void maintain_cutoff(LatencyBuffer *buf,
                     size_t W);

void buffered_read(LatencyBuffer *buf,
                   size_t bytes);

#endif
