/* v92_router_and_latency.c
   ------------------------------------------------------------
   v.92 tone mapping
   Router send/receive stubs
   Latency arbitrage buffer (RTT → W → hidden delay)
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "modem_and_morse.c"

typedef struct {
    uint8_t tone_id;
} V92Tone;

typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   read_pos;
    size_t   frontier;
} LatencyBuffer;

/* --- Morse ↔ v.92 --- */
V92Tone *morse_to_v92(const MorseSymbol *m, size_t n, size_t *out_n) {
    V92Tone *tones = malloc(sizeof(V92Tone) * n);
    for (size_t i = 0; i < n; i++) {
        tones[i].tone_id = (m[i].symbol == '.') ? 1 : 2;
    }
    *out_n = n;
    printf("[v.92] Encoded %zu Morse symbols to tones.\n", n);
    return tones;
}

MorseSymbol *v92_to_morse(const V92Tone *tones, size_t n, size_t *out_n) {
    MorseSymbol *m = malloc(sizeof(MorseSymbol) * n);
    for (size_t i = 0; i < n; i++) {
        m[i].symbol = (tones[i].tone_id == 1) ? '.' : '-';
    }
    *out_n = n;
    printf("[v.92] Decoded %zu tones to Morse.\n", n);
    return m;
}

/* --- router send/receive stubs --- */
void send_v92_over_router(const V92Tone *tones, size_t n) {
    printf("[Router] Sending %zu v.92 tones...\n", n);
}

V92Tone *recv_v92_from_router(size_t *out_n) {
    size_t n = 16;
    V92Tone *tones = malloc(sizeof(V92Tone) * n);
    for (size_t i = 0; i < n; i++) tones[i].tone_id = (i % 2) ? 1 : 2;
    *out_n = n;
    printf("[Router] Received %zu v.92 tones.\n", n);
    return tones;
}

/* --- latency arbitrage buffer --- */
double measure_rtt_s(void) {
    return 0.6; /* 600 ms example */
}

size_t compute_window_bytes(double bandwidth_bps, double rtt_s) {
    double bdp_bits  = bandwidth_bps * rtt_s;
    double bdp_bytes = bdp_bits / 8.0;
    return (size_t)bdp_bytes;
}

void maintain_cutoff(LatencyBuffer *buf, size_t W) {
    if (buf->frontier < buf->read_pos + W) {
        size_t need = buf->read_pos + W - buf->frontier;
        buf->frontier += need;
        if (buf->frontier > buf->len) buf->frontier = buf->len;
    }
}

void buffered_read(LatencyBuffer *buf, size_t bytes) {
    if (buf->read_pos + bytes > buf->frontier) {
        printf("[Latency] App would hit latency here.\n");
        return;
    }
    buf->read_pos += bytes;
    printf("[Latency] App read %zu bytes; frontier=%zu.\n",
           bytes, buf->frontier);
}
