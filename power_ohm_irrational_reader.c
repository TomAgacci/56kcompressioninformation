/* power_ohm_irrational_reader.c
   ------------------------------------------------------------
   Reads a tiny ohm/power value,
   maps it to a varying decimal,
   and uses that as a data key in RAM.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define ADC_MAX        4095        // 12-bit ADC
#define BASE_SCALE     1e-12       // tiny base scale for decimal
#define POWER_EXP_BASE 1.000001    // slight exponential variation

/* --- stub: read power/ohm from "meter" (ADC) --- */
uint16_t read_power_adc(void) {
    /* In real hardware: read from ADC channel. */
    static uint16_t v = 1000;
    v = (v + 137) & ADC_MAX;       // pseudo-changing value
    return v;
}

/* --- map ADC reading to tiny "irrational-like" decimal --- */
double adc_to_irrational_decimal(uint16_t adc) {
    /* Normalize to [0,1] */
    double norm = (double)adc / (double)ADC_MAX;

    /* Vary by power level using a tiny exponential factor */
    double power_factor = 1.0;
    for (int i = 0; i < 8; i++) {
        power_factor *= POWER_EXP_BASE;
    }

    /* Tiny decimal: base scale * normalized * power factor */
    double dec = BASE_SCALE * norm * power_factor;

    return dec;
}

/* --- use decimal as a key into RAM buffer --- */
uint8_t read_data_from_ram(double key, uint8_t *ram, size_t len) {
    /* Map decimal to an index */
    double scaled = key * 1e15;                // bring into larger range
    size_t idx = (size_t)scaled % len;
    return ram[idx];
}

int main(void) {
    /* Example RAM buffer */
    size_t len = 1024;
    uint8_t *ram = malloc(len);
    for (size_t i = 0; i < len; i++) ram[i] = (uint8_t)(i & 0xFF);

    /* Read power/ohm, convert to decimal, use as key */
    for (int i = 0; i < 5; i++) {
        uint16_t adc = read_power_adc();
        double dec = adc_to_irrational_decimal(adc);
        uint8_t val = read_data_from_ram(dec, ram, len);

        printf("ADC=%4u → decimal=%.18f → RAM[%u]=0x%02X\n",
               adc, dec, (unsigned)((size_t)(dec * 1e15) % len), val);
    }

    free(ram);
    return 0;
}
