/* ============================================================
   PULSE‑OHM / .PCF / 56K FORMULAS
   ============================================================ */

/* ------------------------------------------------------------
   BASIC SIZE & BIT CONVERSIONS
   ------------------------------------------------------------ */
bits   = bytes * 8;
bytes  = bits / 8;

/* ------------------------------------------------------------
   COMPRESSION RATIO
   ------------------------------------------------------------ */
compression_ratio = original_size_bytes / compressed_size_bytes;

/* ------------------------------------------------------------
   COMPRESSED SIZE (given ratio)
   ------------------------------------------------------------ */
compressed_size_bytes = original_size_bytes / compression_ratio;

/* ------------------------------------------------------------
   TRANSFER TIME OVER 56K LINE
   ------------------------------------------------------------ */
transfer_time_seconds = compressed_size_bits / line_rate_bps;

/* Example:
   compressed_size_bits = compressed_size_bytes * 8;
   line_rate_bps = 56000;
*/

/* ------------------------------------------------------------
   EFFECTIVE UNCOMPRESSED THROUGHPUT
   ------------------------------------------------------------ */
effective_bps  = original_size_bits / transfer_time_seconds;
effective_gbps = effective_bps / 1e9;

/* ------------------------------------------------------------
   MULTI‑PATTERN COMPRESSION (RLE-style)
   ------------------------------------------------------------ */
/* For each pattern i:
   pattern[i].value = repeated_byte;
   pattern[i].count = run_length;

   Total compressed size:
*/
compressed_size_bytes =
      sizeof(num_patterns)
    + num_patterns * (sizeof(value) + sizeof(count) + sizeof(pulse_code));

/* Total uncompressed size reconstructed: */
original_size_bytes = sum(pattern[i].count);

/* ------------------------------------------------------------
   400‑TONE MODEM BIT MAPPING
   ------------------------------------------------------------ */
NUM_TONES      = 400;
BITS_PER_TONE  = k;      /* e.g., 1, 2, or 4 bits per tone */
FRAME_BITS     = NUM_TONES * BITS_PER_TONE;

/* Bits consumed per frame: */
used_bits = FRAME_BITS;

/* Number of frames needed for bitstream: */
num_frames = ceil(total_bits / FRAME_BITS);

/* ------------------------------------------------------------
   EFFECTIVE RATE WITH MULTI‑PATTERN + 400 TONES
   ------------------------------------------------------------ */
/* Total compressed bits sent: */
compressed_bits = num_frames * FRAME_BITS;

/* Transfer time: */
transfer_time_seconds = compressed_bits / line_rate_bps;

/* Effective uncompressed throughput: */
effective_bps  = original_size_bits / transfer_time_seconds;
effective_gbps = effective_bps / 1e9;

/* ============================================================
   SUMMARY OF KEY FORMULAS
   ------------------------------------------------------------
   bits = bytes * 8
   CR = original_size / compressed_size
   compressed_size = original_size / CR
   transfer_time = compressed_bits / line_rate
   effective_rate = original_bits / transfer_time
   effective_gbps = effective_rate / 1e9
   ============================================================ */
