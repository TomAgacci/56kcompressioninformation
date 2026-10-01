/* ============================================================
   pcf_container_spec.h  (Minimal Spec in C-Style Comments)
   ------------------------------------------------------------
   PCF (Pulse Container Format) — v1.0
   ============================================================ */

/*
PCF FILE LAYOUT (little-endian):

[Header]
  uint32_t magic;        // "PCF1" = 0x50434631
  uint32_t version;      // e.g., 0x00010000 for v1.0
  uint32_t num_entries;  // number of pattern descriptors

[Entries] (repeated num_entries times)
  uint8_t  value;        // repeated byte
  uint32_t count;        // run length
  uint32_t pulse_code;   // timing / pattern ID

Notes:
  • This is a simple RLE-style container tuned for pulse‑ohm.
  • magic identifies the file as PCF.
  • version allows future extension.
  • num_entries tells how many descriptors follow.
  • Each descriptor reconstructs 'count' copies of 'value'.
  • pulse_code can be used by the modem/timing layer.
*/
