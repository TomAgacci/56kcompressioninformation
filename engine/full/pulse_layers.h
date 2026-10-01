/* pulse_layers.h
 *
 * Multi-layer pulse compression on top of PulseSymbol.
 * Layer 0: raw pulses (duration, power)
 * Layer 1: quantized pulse symbols
 * Layer 2: RLE-compressed pulse symbols
 * Layer 3: meta-pulses encoding compressed stream back into pulses
 */

#ifndef PULSE_LAYERS_H
#define PULSE_LAYERS_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    double duration_s;
    double power_level;
} PulseSymbol;

/* Layer 1: quantized pulse symbol */
typedef struct {
    uint16_t code;   /* quantized duration index */
    uint16_t power;  /* quantized power index    */
} PulseCode;

/* Layer 2: RLE-compressed pulse codes */
typedef struct {
    PulseCode code;
    uint32_t  run_len;
} PulseRLE;

/* Layer 3: meta-pulse (compressed representation back into pulses) */
typedef struct {
    double duration_s;
    double power_level;
} MetaPulse;

/* Quantize raw pulses into PulseCode (Layer 1) */
PulseCode *quantize_pulses_to_codes(const PulseSymbol *pulses,
                                    size_t n,
                                    size_t *out_n);

/* Compress PulseCode stream with simple RLE (Layer 2) */
PulseRLE *compress_codes_rle(const PulseCode *codes,
                             size_t n,
                             size_t *out_n);

/* Expand RLE back to PulseCode (for decoding) */
PulseCode *expand_rle_to_codes(const PulseRLE *rle,
                               size_t n,
                               size_t *out_n);

/* Encode compressed RLE stream into MetaPulse (Layer 3) */
MetaPulse *encode_rle_to_meta_pulses(const PulseRLE *rle,
                                     size_t n,
                                     size_t *out_n);

/* Decode MetaPulse back into PulseRLE (inverse of Layer 3) */
PulseRLE *decode_meta_pulses_to_rle(const MetaPulse *meta,
                                    size_t n,
                                    size_t *out_n);

/* Utility: reconstruct approximate PulseSymbol from PulseCode */
PulseSymbol *codes_to_pulses(const PulseCode *codes,
                             size_t n,
                             size_t *out_n);

#endif
