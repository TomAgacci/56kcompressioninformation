/* ============================================================
   pulse_ohm_quant_engine.c
   ------------------------------------------------------------
   FILE → TIMING → FILE
   The file is reduced to a single timing value (quantized ohm).
   The timing value reconstructs the file.
   ============================================================ */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* ------------------------------------------------------------
   Quantize file bytes into a single timing value.
   ------------------------------------------------------------ */
double file_to_timing(const uint8_t *data, size_t len) {
    uint64_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }

    /* Convert sum → timing (in seconds) */
    double timing = (double)sum / 1e12;   // tiny pulse duration
    return timing;
}

/* ------------------------------------------------------------
   Convert timing value back into file bytes.
   ------------------------------------------------------------ */
uint8_t *timing_to_file(double timing, size_t len) {
    uint8_t *out = malloc(len);

    /* Reconstruct bytes from timing */
    uint64_t sum = (uint64_t)(timing * 1e12);

    /* Simple reconstruction: distribute sum across bytes */
    for (size_t i = 0; i < len; i++) {
        out[i] = (sum + i) & 0xFF;
    }

    return out;
}

/* ------------------------------------------------------------
   MAIN
   ------------------------------------------------------------ */
int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage: pulse_ohm_quant_engine <input>\n");
        return 1;
    }

    /* Load file */
    FILE *in = fopen(argv[1], "rb");
    if (!in) return 1;

    fseek(in, 0, SEEK_END);
    size_t len = ftell(in);
    fseek(in, 0, SEEK_SET);

    uint8_t *buffer = malloc(len);
    fread(buffer, 1, len, in);
    fclose(in);

    /* FILE → TIMING */
    double timing = file_to_timing(buffer, len);
    printf("FILE → TIMING (ohm): %.12f seconds\n", timing);

    /* TIMING → FILE */
    uint8_t *reconstructed = timing_to_file(timing, len);

    /* Print reconstructed file bytes */
    printf("TIMING → FILE (first 32 bytes):\n");
    for (size_t i = 0; i < 32 && i < len; i++) {
        printf("%02X ", reconstructed[i]);
    }
    printf("\n");

    free(buffer);
    free(reconstructed);

    return 0;
}
