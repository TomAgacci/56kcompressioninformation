/* morse_pulse_layer.c
   ------------------------------------------------------------
   Encodes bytes into Morse-like pulse packets and decodes them.
   This sits under your 56k compressor and above RF / audio.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t symbol;   // dot/dash pattern encoded as bits
    uint8_t length;   // number of elements in symbol
} MorseSymbol;

/* Very simple: map 4-bit nibbles to Morse-like symbols */
MorseSymbol encode_nibble_to_morse(uint8_t nibble) {
    MorseSymbol m;
    m.length = 4;
    m.symbol = nibble & 0x0F;  // 4 bits → 4 pulses (dot/dash)
    return m;
}

/* Encode a byte stream into Morse pulse packets */
size_t encode_bytes_to_morse(const uint8_t *data, size_t len, MorseSymbol **out) {
    MorseSymbol *symbols = malloc(sizeof(MorseSymbol) * len * 2); // 2 nibbles per byte
    size_t n = 0;

    for (size_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        symbols[n++] = encode_nibble_to_morse((b >> 4) & 0x0F);
        symbols[n++] = encode_nibble_to_morse(b & 0x0F);
    }

    *out = symbols;
    return n;
}

/* Placeholder: send Morse pulses over underlying medium */
void send_morse_symbol(const MorseSymbol *m) {
    (void)m;
    /* Real implementation:
       - map bits in m->symbol to short/long pulses
       - key RF / audio / GPIO accordingly
    */
}
