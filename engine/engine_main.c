/* engine_main.c
   ------------------------------------------------------------
   Full pipeline:

   FILE → pulse-ohm → timing/pulses → 56k → Morse → v.92 → router
   → v.92 → Morse → 56k → pulses → patterns → FILE (RAM)

   Plus latency arbitrage demo.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "v92_router_and_latency.c"

/* --- patterns → file (RAM) --- */
uint8_t *patterns_to_file(const PulsePattern *patterns, size_t n,
                          size_t *out_len)
{
    size_t total = 0;
    for (size_t i = 0; i < n; i++) total += patterns[i].count;

    uint8_t *buf = malloc(total);
    size_t pos = 0;

    for (size_t i = 0; i < n; i++) {
        memset(buf + pos, patterns[i].value, patterns[i].count);
        pos += patterns[i].count;
    }

    *out_len = total;
    printf("[RAM] Expanded patterns into %zu bytes.\n", total);
    return buf;
}

/* --- pulses → patterns (simple mapping) --- */
PulsePattern *pulses_to_patterns(const PulseSymbol *pulses, size_t n,
                                 uint32_t max_count, size_t *out_n)
{
    PulsePattern *patterns = malloc(sizeof(PulsePattern) * n);
    for (size_t i = 0; i < n; i++) {
        double g = (pulses[i].duration_s <=
                    (MIN_DURATION_S + MAX_DURATION_S) / 2.0) ? 0.25 : 0.75;
        patterns[i].value      = (pulses[i].duration_s <=
                                  (MIN_DURATION_S + MAX_DURATION_S) / 2.0) ? 0x00 : 0xFF;
        patterns[i].count      = (uint32_t)(g * max_count);
        patterns[i].pulse_code = 0x56000000 | (i & 0xFFFF);
    }
    *out_n = n;
    printf("[Pulse-ohm] Reconstructed %zu patterns from pulses.\n", n);
    return patterns;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: engine_main <input_file>\n");
        return 1;
    }

    /* Load input file */
    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("open");
        return 1;
    }

    fseek(in, 0, SEEK_END);
    size_t len = ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t *buffer = malloc(len);
    fread(buffer, 1, len, in);
    fclose(in);

    printf("Loaded file: %zu bytes.\n", len);

    /* File-size gradient → base power/timing */
    size_t min_bytes = 1024;
    size_t max_bytes = 100 * 1024 * 1024;
    double g_file    = size_to_gradient(len, min_bytes, max_bytes);
    double base_power   = gradient_to_power(g_file);
    double base_duration = gradient_to_duration(g_file);

    printf("[Gradient] g=%.6f, base_power=%.6f, base_duration=%.9f s\n",
           g_file, base_power, base_duration);

    /* FILE → pulse-ohm patterns */
    PulsePattern *patterns = NULL;
    size_t n_patterns = compress_to_pulse_patterns(buffer, len, &patterns);
    printf("[Pulse-ohm] Compressed into %zu patterns.\n", n_patterns);

    /* PATTERNS → timing/pulse symbols */
    uint32_t max_count = (uint32_t)len;
    size_t n_pulses = 0;
    PulseSymbol *pulses = patterns_to_pulses(patterns, n_patterns,
                                             max_count,
                                             base_power,
                                             base_duration,
                                             &n_pulses);
    printf("[Timing] Mapped patterns to %zu pulse symbols.\n", n_pulses);

    /* SEND: pulses → 56k → Morse → v.92 → router */
    send_pulses_over_56k_phonic(pulses, n_pulses);
    size_t n_morse = 0;
    MorseSymbol *morse = pulses_to_morse(pulses, n_pulses, &n_morse);
    size_t n_tones = 0;
    V92Tone *tones = morse_to_v92(morse, n_morse, &n_tones);
    send_v92_over_router(tones, n_tones);

    /* RECV: router → v.92 → Morse → 56k → pulses */
    size_t n_tones_recv = 0;
    V92Tone *tones_recv = recv_v92_from_router(&n_tones_recv);
    size_t n_morse_recv = 0;
    MorseSymbol *morse_recv = v92_to_morse(tones_recv, n_tones_recv,
                                           &n_morse_recv);
    size_t n_pulses_recv = 0;
    PulseSymbol *pulses_recv = morse_to_pulses(morse_recv, n_morse_recv,
                                               &n_pulses_recv);

    /* PULSES → patterns → FILE (RAM) */
    size_t n_patterns_recv = 0;
    PulsePattern *patterns_recv = pulses_to_patterns(pulses_recv,
                                                     n_pulses_recv,
                                                     max_count,
                                                     &n_patterns_recv);
    size_t out_len = 0;
    uint8_t *file_out = patterns_to_file(patterns_recv,
                                         n_patterns_recv,
                                         &out_len);

    printf("[Final] Reconstructed file size (RAM): %zu bytes.\n", out_len);

    /* LATENCY ARBITRAGE DEMO */
    double rtt_s = measure_rtt_s();
    double bw_bps = 20e6;
    size_t W = compute_window_bytes(bw_bps, rtt_s);

    printf("[Latency] RTT=%.3f s, BW=%.1f Mbps, W=%zu bytes\n",
           rtt_s, bw_bps / 1e6, W);

    LatencyBuffer buf;
    buf.len      = W * 2;
    buf.data     = malloc(buf.len);
    buf.read_pos = 0;
    buf.frontier = 0;

    maintain_cutoff(&buf, W);
    for (int i = 0; i < 4; i++) {
        buffered_read(&buf, W / 4);
        maintain_cutoff(&buf, W);
    }

    /* Cleanup */
    free(buffer);
    free(patterns);
    free(pulses);
    free(morse);
    free(tones);
    free(tones_recv);
    free(morse_recv);
    free(pulses_recv);
    free(patterns_recv);
    free(file_out);
    free(buf.data);

    return 0;
}
