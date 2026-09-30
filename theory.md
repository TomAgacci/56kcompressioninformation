/* ============================================================
   Pulse‑Ohm Compression Descriptor (.pcf / .pulse)
   ------------------------------------------------------------
   This example shows how a 100 MB patterned file compresses
   into a tiny pulse‑timing descriptor. The comments explain
   each field and how the effective transfer speed is computed.
   ============================================================ */

/* ------------------------------------------------------------
   ORIGINAL FILE INFORMATION
   ------------------------------------------------------------ */
original_size_bytes      = 100000000;     // 100 MB file
original_entropy_bits    = 0;             // blank/patterned → fully compressible

/* ------------------------------------------------------------
   COMPRESSED OUTPUT (.pcf / .pulse)
   ------------------------------------------------------------ */
compressed_size_bytes    = 200;           // typical for blank/patterned data
compressed_size_bits     = compressed_size_bytes * 8;

/* ------------------------------------------------------------
   CHANNEL INFORMATION (Pulse‑Ohm Audio Modem)
   ------------------------------------------------------------ */
channel_rate_bps         = 30000;         // 30 kbps audio link (near Shannon limit)

/* ------------------------------------------------------------
   TRANSFER TIME (compressed)
   ------------------------------------------------------------ */
transfer_time_seconds    = compressed_size_bits / channel_rate_bps;
// Example: 200 bytes → 1600 bits → 1600 / 30000 ≈ 0.053 seconds

/* ------------------------------------------------------------
   EFFECTIVE UNCOMPRESSED TRANSFER RATE
   ------------------------------------------------------------
   This is the “felt speed” once the receiver reconstructs the
   full 100 MB file from the tiny .pcf/.pulse descriptor.
   ------------------------------------------------------------ */
effective_rate_bps       = original_size_bytes * 8 / transfer_time_seconds;
// Example: 100 MB / 0.053s ≈ 15,094,000,000 bps (≈ 15 Gb/s)

/* ------------------------------------------------------------
   SUMMARY
   ------------------------------------------------------------
   - A 100 MB patterned file compresses to ~200 bytes.
   - Over a 30 kbps audio channel, it transfers in ~0.053 seconds.
   - After decompression, the effective data rate is ~15 Gb/s.
   - This only works for extremely redundant data.
   - Random data will NOT compress and stays ~100 MB.
   ------------------------------------------------------------ */
