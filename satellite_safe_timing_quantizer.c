/* satellite_safe_timing_quantizer.c
   ------------------------------------------------------------
   Quantizes pulse durations into safe, noise‑resistant bins
   suitable for satellite / long‑range RF paths.
*/

#include <stdio.h>
#include <stdint.h>

#define BITS_PER_PULSE   8          // more conservative for noise
#define STEP_DURATION_S  5e-9       // 5 ns steps
#define TOLERANCE_S      3e-9       // ±3 ns tolerance

/* Quantize measured duration to nearest symbol */
int quantize_duration(double measured, uint16_t *symbol_out) {
    double idx = measured / STEP_DURATION_S;
    uint16_t s = (uint16_t)(idx + 0.5);

    double ideal = (double)s * STEP_DURATION_S;
    double err = measured - ideal;

    if (err > TOLERANCE_S || err < -TOLERANCE_S) {
        return 0; // too noisy / ambiguous
    }

    *symbol_out = s & ((1u << BITS_PER_PULSE) - 1);
    return 1;
}
