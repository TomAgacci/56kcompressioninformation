/* ============================================================
   client_56k_morse_sat_buffer.c
   ------------------------------------------------------------
   CLIENT SIDE:
   - Receives patterns over 56k→Morse→satellite
   - Buffers predicted data
   - Serves requests from local buffer
   ============================================================ */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t  value;
    uint32_t count;
    uint32_t pulse_code;
} PulsePattern;

typedef struct {
    uint8_t *data;
    size_t   len;
} PredictiveBuffer;

/* --- Stub: receive pattern from satellite/Morse/56k --- */
int receive_pattern_from_satellite(PulsePattern *p) {
    /* Real implementation:
       - read from socket / modem / Morse decoder
       - fill p->value, p->count, p->pulse_code
    */
    static int calls = 0;
    if (calls++ > 8) return 0;  // simulate end

    p->value      = 0x00;
    p->count      = 1024 * calls;   // growing zeros
    p->pulse_code = 0x56000000 | calls;
    return 1;
}

/* --- Expand pattern into buffer --- */
void append_pattern_to_buffer(PredictiveBuffer *buf, PulsePattern p) {
    buf->data = realloc(buf->data, buf->len + p.count);
    memset(buf->data + buf->len, p.value, p.count);
    buf->len += p.count;
}

/* --- Serve a request from buffer --- */
void serve_request_from_buffer(PredictiveBuffer *buf, size_t offset, size_t length) {
    if (offset + length > buf->len) {
        printf("Client: request beyond buffer, would need live fetch.\n");
        return;
    }
    printf("Client: served %zu bytes from predictive buffer at offset %zu.\n", length, offset);
    /* In real code, you'd copy to user/application here. */
}

int main(void) {
    PredictiveBuffer buf = { NULL, 0 };

    /* Receive speculative patterns and build buffer */
    PulsePattern p;
    while (receive_pattern_from_satellite(&p)) {
        append_pattern_to_buffer(&buf, p);
    }

    printf("Client: predictive buffer filled with %zu bytes.\n", buf.len);

    /* Simulated user requests that arrive later */
    serve_request_from_buffer(&buf, 0, 4096);
    serve_request_from_buffer(&buf, 8192, 4096);

    free(buf.data);
    return 0;
}
