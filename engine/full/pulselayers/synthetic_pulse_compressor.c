/* synthetic_pulse_compressor.c
 *
 * Demonstrates synthetic 120x and 500x “compression” in the pulse domain
 * by collapsing long, perfectly-structured pulse runs into single meta-pulses.
 *
 * This is NOT general-purpose compression — it’s synthetic, toy-style:
 *  - Input: N identical pulses (same duration/power)
 *  - Output: 1 meta-pulse that encodes “pattern + run length”
 *  - Compression ratio: N : 1  (e.g., 120:1, 500:1)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Base pulse symbol (Layer 0) */
typedef struct {
    double duration_s;
    double power_level;
} PulseSymbol;

/* Meta-pulse that encodes a whole run (Layer 3 synthetic) */
typedef struct {
    double duration_s;   /* encodes run length */
    double power_level;  /* encodes pattern ID */
} MetaPulse;

/* Synthetic pattern ID */
#define PATTERN_ID_120   1
#define PATTERN_ID_500   2

/* Scaling constants for encoding run length into duration */
#define BASE_DUR_S       1e-6
#define DUR_SCALE_S      1e-6

/* Encode a synthetic run of identical pulses into a single meta-pulse */
MetaPulse encode_run_to_meta(uint32_t run_len, uint32_t pattern_id)
{
    MetaPulse m;
    m.duration_s  = BASE_DUR_S + (double)run_len * DUR_SCALE_S;
    m.power_level = (double)pattern_id; /* simple synthetic mapping */
    return m;
}

/* Decode meta-pulse back into run length + pattern ID */
void decode_meta_to_run(const MetaPulse *m,
                        uint32_t *out_run_len,
                        uint32_t *out_pattern_id)
{
    double run_f = (m->duration_s - BASE_DUR_S) / DUR_SCALE_S;
    if (run_f < 1.0) run_f = 1.0;
    *out_run_len    = (uint32_t)(run_f + 0.5);
    *out_pattern_id = (uint32_t)(m->power_level + 0.5);
}

/* Generate synthetic input: N identical pulses */
PulseSymbol *generate_synthetic_pulses(uint32_t run_len,
                                       double duration_s,
                                       double power_level)
{
    PulseSymbol *p = (PulseSymbol *)malloc(sizeof(PulseSymbol) * run_len);
    if (!p) return NULL;

    for (uint32_t i = 0; i < run_len; i++) {
        p[i].duration_s  = duration_s;
        p[i].power_level = power_level;
    }
    return p;
}

/* Compute compression ratio: input_count / output_count */
double compression_ratio(uint32_t input_count, uint32_t output_count)
{
    if (output_count == 0) return 0.0;
    return (double)input_count / (double)output_count;
}

int main(void)
{
    /* --- Synthetic 120x case --- */
    uint32_t run_120 = 120;
    PulseSymbol *synthetic_120 =
        generate_synthetic_pulses(run_120, 2e-6, 0.5);

    /* Collapse 120 identical pulses into 1 meta-pulse */
    MetaPulse meta_120 = encode_run_to_meta(run_120, PATTERN_ID_120);

    /* Decode back to verify */
    uint32_t decoded_run_120 = 0, decoded_pattern_120 = 0;
    decode_meta_to_run(&meta_120, &decoded_run_120, &decoded_pattern_120);

    double ratio_120 = compression_ratio(run_120, 1);

    printf("Synthetic 120x example:\n");
    printf("  Input pulses:      %u\n", run_120);
    printf("  Output meta-pulses: 1\n");
    printf("  Compression ratio: %.1fx\n", ratio_120);
    printf("  Decoded run_len:   %u\n", decoded_run_120);
    printf("  Decoded pattern_id:%u\n\n", decoded_pattern_120);

    free(synthetic_120);

    /* --- Synthetic 500x case --- */
    uint32_t run_500 = 500;
    PulseSymbol *synthetic_500 =
        generate_synthetic_pulses(run_500, 3e-6, 0.7);

    /* Collapse 500 identical pulses into 1 meta-pulse */
    MetaPulse meta_500 = encode_run_to_meta(run_500, PATTERN_ID_500);

    /* Decode back to verify */
    uint32_t decoded_run_500 = 0, decoded_pattern_500 = 0;
    decode_meta_to_run(&meta_500, &decoded_run_500, &decoded_pattern_500);

    double ratio_500 = compression_ratio(run_500, 1);

    printf("Synthetic 500x example:\n");
    printf("  Input pulses:      %u\n", run_500);
    printf("  Output meta-pulses: 1\n");
    printf("  Compression ratio: %.1fx\n", ratio_500);
    printf("  Decoded run_len:   %u\n", decoded_run_500);
    printf("  Decoded pattern_id:%u\n\n", decoded_pattern_500);

    free(synthetic_500);

    return 0;
}
