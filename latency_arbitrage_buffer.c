/* latency_arbitrage_buffer.c
   ------------------------------------------------------------
   Measures RTT, computes bandwidth-delay window W,
   and keeps a buffer cutoff so reads don't see satellite latency.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    uint8_t *data;
    size_t   len;        // total bytes in buffer
    size_t   read_pos;   // where the application is reading
    size_t   frontier;   // how far ahead data is filled
} LatencyBuffer;

/* --- get current time in seconds --- */
double now_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* --- measure RTT (stub: pretend satellite) --- */
double measure_rtt_s(void) {
    /* In real code: send a ping, wait for reply, measure delta. */
    return 0.6;  // 600 ms RTT example
}

/* --- compute window W = bandwidth * RTT --- */
size_t compute_window_bytes(double bandwidth_bps, double rtt_s) {
    double bdp_bits  = bandwidth_bps * rtt_s;
    double bdp_bytes = bdp_bits / 8.0;
    return (size_t)bdp_bytes;
}

/* --- ensure frontier is at least W ahead of read_pos --- */
void maintain_cutoff(LatencyBuffer *buf, size_t W) {
    if (buf->frontier < buf->read_pos + W) {
        size_t need = buf->read_pos + W - buf->frontier;
        /* In real code: fetch 'need' bytes from satellite/56k/Morse here. */
        buf->frontier += need;
        if (buf->frontier > buf->len) buf->frontier = buf->len;
    }
}

/* --- simulate a read that never sees latency --- */
void buffered_read(LatencyBuffer *buf, size_t bytes) {
    if (buf->read_pos + bytes > buf->frontier) {
        printf("App would hit latency here.\n");
        return;
    }
    buf->read_pos += bytes;
    printf("App read %zu bytes; frontier at %zu, window kept.\n",
           bytes, buf->frontier);
}

int main(void) {
    /* Start: timing for the data from point of summon */
    double t_start = now_s();

    /* Measure RTT and compute W */
    double rtt_s = measure_rtt_s();          // ~0.6 s
    double bw_bps = 20e6;                    // 20 Mbps satellite-equivalent
    size_t W = compute_window_bytes(bw_bps, rtt_s);

    printf("RTT: %.3f s, bandwidth: %.1f Mbps, window W: %zu bytes\n",
           rtt_s, bw_bps / 1e6, W);

    /* Initialize buffer */
    LatencyBuffer buf;
    buf.len      = W * 4;                    // total capacity
    buf.data     = malloc(buf.len);
    buf.read_pos = 0;
    buf.frontier = 0;

    /* Fill initial window so the app starts "inside" the buffer */
    maintain_cutoff(&buf, W);

    /* Metronomic irrational after the decimal: we just keep time moving
       and keep the frontier ahead of the read point. */
    for (int i = 0; i < 5; i++) {
        buffered_read(&buf, W / 4);          // app consumes a quarter-window
        maintain_cutoff(&buf, W);            // we advance frontier to keep W
    }

    double t_end = now_s();
    printf("Total wall time: %.3f s (app never saw satellite RTT).\n",
           t_end - t_start);

    free(buf.data);
    return 0;
}
