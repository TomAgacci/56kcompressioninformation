#ifndef COMPRESSION_AND_TIMING_H
#define COMPRESSION_AND_TIMING_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    PATTERN_RUN,
    PATTERN_LITERAL
} PatternType;

typedef struct {
    PatternType type;
    uint8_t     value;
    uint32_t    count;
    uint32_t    pulse_code;
    uint8_t    *literal;
} PulsePattern;

typedef struct {
    double duration_s;
    double power_level;
} PulseSymbol;

size_t compress_to_pulse_patterns(const uint8_t *data,
                                  size_t len,
                                  PulsePattern **out);

double size_to_gradient(size_t size_bytes,
                        size_t min_bytes,
                        size_t max_bytes);

double gradient_to_power(double g);
double gradient_to_duration(double g);

PulseSymbol *patterns_to_pulses(const PulsePattern *patterns,
                                size_t n,
                                uint32_t max_count,
                                double base_power,
                                double base_duration,
                                size_t *out_n);

size_t encode_bytes_to_pulses(const uint8_t *data,
                              size_t len,
                              double **out);

int quantize_duration(double measured,
                      uint16_t *symbol_out);

#endif
