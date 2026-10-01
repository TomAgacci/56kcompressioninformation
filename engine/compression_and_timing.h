#ifndef COMPRESSION_AND_TIMING_H
#define COMPRESSION_AND_TIMING_H

#include <stdint.h>
#include <stdlib.h>

/* ------------------------------
   Pattern Types
------------------------------ */
typedef enum {
    PATTERN_RUN,
    PATTERN_LITERAL
} PatternType;

/* ------------------------------
   Pulse-Ohm Pattern
------------------------------ */
typedef struct {
    PatternType type;
    uint8_t     value;
    uint32_t    count;
    uint32_t    pulse_code;
    uint8_t    *literal;
} PulsePattern;

/* ------------------------------
   Pulse Symbol (Timing + Power)
------------------------------ */
typedef struct {
    double duration_s;
    double power_level;
} PulseSymbol;

/* ------------------------------
   Compression API
------------------------------ */
size_t compress_to_pulse_patterns(const uint8_t *data,
                                  size_t len,
                                  PulsePattern **out);

/* ------------------------------
   File-size → Gradient
------------------------------ */
double size_to_gradient(size_t size_bytes,
                        size_t min_bytes,
                        size_t max_bytes);

double gradient_to_power(double g);
double gradient_to_duration(double g);

/* ------------------------------
   Patterns → Pulse Symbols
------------------------------ */
PulseSymbol *patterns_to_pulses(const PulsePattern *patterns,
                                size_t n,
                                uint32_t max_count,
                                double base_power,
                                double base_duration,
                                size_t *out_n);

/* ------------------------------
   Multi-Pulse Timing Encoder
------------------------------ */
size_t encode_bytes_to_pulses(const uint8_t *data,
                              size_t len,
                              double **out);

/* ------------------------------
   Satellite-Safe Quantizer
------------------------------ */
int quantize_duration(double measured,
                      uint16_t *symbol_out);

#endif
