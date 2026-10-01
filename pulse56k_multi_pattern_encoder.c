/* ============================================================
   pulse56k_multi_pattern_encoder.c
   ------------------------------------------------------------
   56k-optimized multi-pattern encoder:
   - Compresses runs
   - Estimates transfer time and effective throughput
   ============================================================ */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t  value;
    uint32_t count;
    uint32_t pulse_code;
} PulsePattern;

size_t multipattern_compress(uint8_t *data, size_t len, PulsePattern **out) {
    PulsePattern *patterns = malloc(sizeof(PulsePattern) * len);
    size_t n = 0;

    if (len == 0) {
        *out = NULL;
        return 0;
    }

    uint8_t  current = data[0];
    uint32_t count   = 1;

    for (size_t i = 1; i < len; i++) {
        if (data[i] == current) {
            count++;
        } else {
            patterns[n].value      = current;
            patterns[n].count      = count;
            patterns[n].pulse_code = 0x56000000 | (n & 0xFFFF);
            n++;

            current = data[i];
            count   = 1;
        }
    }

    patterns[n].value      = current;
    patterns[n].count      = count;
    patterns[n].pulse_code = 0x56000000 | (n & 0xFFFF);
    n++;

    *out = patterns;
    return n;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: pulse56k_multi_pattern_encoder <input>\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) return 1;

    fseek(in, 0, SEEK_END);
    size_t len = ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t *buffer = malloc(len);
    fread(buffer, 1, len, in);
    fclose(in);

    PulsePattern *patterns = NULL;
    size_t n = multipattern_compress(buffer, len, &patterns);

    /* Estimate compressed size */
    size_t compressed_bytes =
        4 + 4 + 4 +                 /* header: magic, version, num_entries */
        n * (1 + 4 + 4);            /* each pattern: value + count + pulse_code */

    double original_bits   = (double)len * 8.0;
    double compressed_bits = (double)compressed_bytes * 8.0;
    double line_rate_bps   = 56000.0;

    double transfer_time_seconds = compressed_bits / line_rate_bps;
    double effective_bps         = original_bits / transfer_time_seconds;
    double effective_gbps        = effective_bps / 1e9;

    printf("Original size:          %zu bytes\n", len);
    printf("Compressed size:        %zu bytes (PCF estimate)\n", compressed_bytes);
    printf("Patterns:               %zu\n", n);
    printf("56k line rate:          %.0f bps\n", line_rate_bps);
    printf("Transfer time:          %.6f s\n", transfer_time_seconds);
    printf("Effective throughput:   %.3f Gbps\n", effective_gbps);

    free(patterns);
    free(buffer);
    return 0;
}
