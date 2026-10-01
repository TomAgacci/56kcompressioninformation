#ifndef PULSE_LAYERS_API_H
#define PULSE_LAYERS_API_H

#include <stdint.h>
#include <stdlib.h>

/* -----------------------------
   Base Pulse Symbol (Layer 0)
   ----------------------------- */
typedef struct {
    double duration_s;
    double power_level;
} PulseSymbol;

/* -----------------------------
   Grammar Motif (Layer 1)
   ----------------------------- */
typedef struct {
    uint8_t motif_id;      /* 0..255 */
    uint8_t pulse_count;   /* number of pulses in motif */
    PulseSymbol pulses[8]; /* max 8 pulses per motif */
} PulseMotif;

/* -----------------------------
   Structuralized Frame (Layer 2)
   ----------------------------- */
typedef struct {
    uint16_t frame_id;     /* hashed motif sequence */
    uint8_t  count;        /* number of motifs */
    PulseMotif motifs[16]; /* max 16 motifs per frame */
} PulseFrame;

/* -----------------------------
   Entropy-Resistant Mapping (Layer 3)
   ----------------------------- */
typedef struct {
    uint32_t map_id;       /* deterministic mapping ID */
    uint32_t run_len;      /* synthetic run length */
    PulseSymbol meta;      /* encoded meta-pulse */
} PulseMapped;

/* -----------------------------
   API Functions
   ----------------------------- */

/* 1. Pulse Grammar Encoder */
PulseMotif *pulse_grammar_encode(const uint8_t *data,
                                 size_t len,
                                 size_t *out_count);

/* 2. Pulse Structuralizer */
PulseFrame *pulse_structuralize(const PulseMotif *motifs,
                                size_t count,
                                size_t *out_frames);

/* 3. Entropy-Resistant Pulse Mapper */
PulseMapped *pulse_map_entropy_resistant(const PulseFrame *frames,
                                         size_t frame_count,
                                         size_t *out_mapped);

#endif
