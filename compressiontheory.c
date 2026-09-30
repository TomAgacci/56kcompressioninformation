/* ============================================================
   pulse56k_effective_speed.c
   ------------------------------------------------------------
   Computes:
     • compressed transfer time over a 56k line
     • effective uncompressed throughput (GB/s)
   For a 100 MB file compressed to a tiny .pcf/.pulse descriptor.
   ============================================================ */

#include <stdio.h>

int main(void) {

    /* --------------------------------------------------------
       INPUT PARAMETERS
       -------------------------------------------------------- */
    double original_size_bytes   = 100.0 * 1024 * 1024;   // 100 MB
    double compressed_size_bytes = 200.0;                 // tiny .pcf descriptor
    double line_rate_bps         = 56000.0;               // 56k line

    /* --------------------------------------------------------
       DERIVED VALUES
       -------------------------------------------------------- */
    double original_bits   = original_size_bytes * 8.0;
    double compressed_bits = compressed_size_bytes * 8.0;

    /* Transfer time for compressed descriptor */
    double transfer_time_seconds = compressed_bits / line_rate_bps;

    /* Effective uncompressed throughput */
    double effective_bps = original_bits / transfer_time_seconds;
    double effective_gbps = effective_bps / 1e9;

    /* --------------------------------------------------------
       OUTPUT
       -------------------------------------------------------- */
    printf("Original size:          %.0f bytes (100 MB)\n", original_size_bytes);
    printf("Compressed size:        %.0f bytes (.pcf/.pulse)\n", compressed_size_bytes);
    printf("Line rate:              %.0f bps (56k)\n\n", line_rate_bps);

    printf("Transfer time:          %.4f seconds\n", transfer_time_seconds);
    printf("Effective throughput:   %.2f Gbps (after decompression)\n", effective_gbps);

    return 0;
}

/* ============================================================
   NOTES:
   ------------------------------------------------------------
   • This demonstrates how a 100 MB patterned file can appear
     to transfer at ~28 Gbps after decompression.
   • The physical line is still only 56 kbps — the “speed gain”
     comes entirely from extreme compression.
   • Real-world data rarely compresses this much.
   ============================================================ */
