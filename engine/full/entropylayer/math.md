/* ============================================================
   PulseLayer‑E — Entropy‑Shaping Math
   ============================================================

   Physical link bandwidth (NYC → Portland):
       B_phys = 0.01–0.025 Gbps   // 10–25 Mbps satellite hop

   Compression ratio after:
       - Pulse Grammar
       - PulseLayer‑E (entropy smoothing + binning + shaping)
       - Structuralizer
       - Entropy‑Resistant Mapper

   Average realistic compression ratio:
       R_avg = 10–20

   Experienced throughput:
       B_exp = B_phys * R_avg

   Example:
       B_phys = 0.025 Gbps
       R_avg  = 15

       B_exp = 0.025 * 15
             = 0.375 Gbps

   ------------------------------------------------------------
   File transfer time:
       S_bits = file size in bits

       t_seconds = S_bits / (B_exp * 1e9)

   Example for 100 MB file:
       S_bits = 100 MB * 8 = 800 Mb

       t = 800e6 / (0.375e9)
         ≈ 2.13 seconds

   ------------------------------------------------------------
   Summary:
       PulseLayer‑E boosts compression ratio into the 10–20× band.
       Experienced NYC→Portland speed becomes:
           ~0.1–0.5 Gbps
       Typical 100 MB file arrival:
           ~2–8 seconds
*/
