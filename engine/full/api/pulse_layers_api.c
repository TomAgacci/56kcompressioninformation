#include "pulse_layers_api.h"
#include <string.h>

/* -----------------------------
   Helper: Make a pulse
   ----------------------------- */
static PulseSymbol mkpulse(double d, double p) {
    PulseSymbol s;
    s.duration_s = d;
    s.power_level = p;
    return s;
}

/* ============================================================
   1. PULSE GRAMMAR ENCODER
   - Byte → 2 nibbles → 4 bit-symbols → pulse motifs
   ============================================================ */
PulseMotif *pulse_grammar_encode(const uint8_t *data,
                                 size_t len,
                                 size_t *out_count)
{
    PulseMotif *motifs = malloc(sizeof(PulseMotif) * len * 2);
    size_t out = 0;

    for (size_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        uint8_t hi = (b >> 4) & 0xF;
        uint8_t lo = b & 0xF;

        /* Each nibble becomes a motif */
        for (int n = 0; n < 2; n++) {
            uint8_t nib = (n == 0 ? hi : lo);

            PulseMotif m;
            m.motif_id = nib;
            m.pulse_count = 4;

            /* Bit-symbol → pulse motif */
            for (int bit = 0; bit < 4; bit++) {
                uint8_t bitval = (nib >> (3 - bit)) & 1;
                if (bitval == 0)
                    m.pulses[bit] = mkpulse(2e-6, 0.3);
                else
                    m.pulses[bit] = mkpulse(4e-6, 0.7);
            }

            motifs[out++] = m;
        }
    }

    *out_count = out;
    return motifs;
}

/* ============================================================
   2. PULSE STRUCTURALIZER
   - Group motifs into fixed-size frames
   - Hash motif IDs → frame_id
   ============================================================ */
PulseFrame *pulse_structuralize(const PulseMotif *motifs,
                                size_t count,
                                size_t *out_frames)
{
    size_t frames = (count + 15) / 16;
    PulseFrame *out = malloc(sizeof(PulseFrame) * frames);

    size_t idx = 0;
    for (size_t f = 0; f < frames; f++) {
        PulseFrame fr;
        fr.count = 0;
        fr.frame_id = 0;

        for (int i = 0; i < 16 && idx < count; i++, idx++) {
            fr.motifs[i] = motifs[idx];
            fr.count++;

            /* Simple hash */
            fr.frame_id = (fr.frame_id * 131) ^ motifs[idx].motif_id;
        }

        out[f] = fr;
    }

    *out_frames = frames;
    return out;
}

/* ============================================================
   3. ENTROPY-RESISTANT PULSE MAPPER
   - Collapse frames into meta-pulses
   - Force structure even for random data
   ============================================================ */
PulseMapped *pulse_map_entropy_resistant(const PulseFrame *frames,
                                         size_t frame_count,
                                         size_t *out_mapped)
{
    PulseMapped *mapped = malloc(sizeof(PulseMapped) * frame_count);

    for (size_t i = 0; i < frame_count; i++) {
        PulseMapped m;

        /* Deterministic mapping ID */
        m.map_id = frames[i].frame_id;

        /* Synthetic run length (forces structure) */
        m.run_len = frames[i].count * 8;

        /* Encode into a meta-pulse */
        m.meta.duration_s = 1e-6 + (double)m.run_len * 1e-6;
        m.meta.power_level = 0.1 + (double)m.map_id * 1e-4;

        mapped[i] = m;
    }

    *out_mapped = frame_count;
    return mapped;
}
