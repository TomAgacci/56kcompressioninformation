#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pulse_layers_api.h"

/* demo payload */
static const uint8_t demo_data[] = {
    0x00, 0xFF, 0x3C, 0x7A, 0x10, 0x20, 0x30, 0x40,
    0x55, 0xAA, 0xC3, 0x5E, 0x99, 0x01, 0x02, 0x03
};

static void send_meta_pulses(const PulseMapped *mapped, size_t count)
{
    printf("Sending %zu meta-pulses over link...\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("  [meta %zu] map_id=%08X run_len=%u dur=%.9f power=%.6f\n",
               i,
               mapped[i].map_id,
               mapped[i].run_len,
               mapped[i].meta.duration_s,
               mapped[i].meta.power_level);
    }
}

static double estimate_experienced_gbps(double physical_gbps,
                                        double compression_ratio)
{
    return physical_gbps * compression_ratio;
}

int main(void)
{
    printf("=== Pulse Pipeline + Entropy Layer Demo ===\n\n");

    size_t len = sizeof(demo_data);
    printf("Input bytes: %zu\n", len);

    /* 1. Grammar: bytes -> motifs */
    size_t motif_count = 0;
    PulseMotif *motifs = pulse_grammar_encode(demo_data, len, &motif_count);
    printf("Grammar produced %zu motifs\n", motif_count);

    /* 2. Entropy Layer: motifs -> entropy-shaped motifs */
    size_t entropy_count = 0;
    EntropyMotif *entropy_motifs = pulse_entropy_layer(motifs, motif_count,
                                                       &entropy_count);
    printf("Entropy layer processed %zu motifs\n", entropy_count);

    /* 3. Structuralizer: entropy motifs -> frames
       (we only use class_id/motif_id; pulses are already shaped) */
    size_t frame_count = 0;
    PulseFrame *frames = pulse_structuralize((PulseMotif *)entropy_motifs,
                                             entropy_count,
                                             &frame_count);
    printf("Structuralizer produced %zu frames\n", frame_count);

    /* 4. Entropy-resistant mapper: frames -> meta-pulses */
    size_t mapped_count = 0;
    PulseMapped *mapped = pulse_map_entropy_resistant(frames,
                                                      frame_count,
                                                      &mapped_count);
    printf("Mapper produced %zu meta-pulses\n\n", mapped_count);

    for (size_t i = 0; i < frame_count; i++) {
        printf("Frame %zu: frame_id=%04X motifs=%u\n",
               i, frames[i].frame_id, frames[i].count);
    }
    printf("\n");

    for (size_t i = 0; i < mapped_count; i++) {
        printf("Mapped %zu: map_id=%08X run_len=%u dur=%.9f power=%.6f\n",
               i,
               mapped[i].map_id,
               mapped[i].run_len,
               mapped[i].meta.duration_s,
               mapped[i].meta.power_level);
    }
    printf("\n");

    send_meta_pulses(mapped, mapped_count);

    double physical_gbps = 0.025;   /* 25 Mbps */
    double avg_ratio     = 15.0;    /* target with PulseLayer-E */
    double exp_speed     = estimate_experienced_gbps(physical_gbps, avg_ratio);

    printf("Estimated experienced speed with entropy layer: %.3f Gbps\n",
           exp_speed);

    free(motifs);
    free(entropy_motifs);
    free(frames);
    free(mapped);

    printf("\nPipeline complete.\n");
    return 0;
}
