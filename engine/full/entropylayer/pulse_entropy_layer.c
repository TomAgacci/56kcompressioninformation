/* pulse_entropy_layer.c
 *
 * PulseLayer-E: entropy-shaping layer between grammar and structuralizer.
 * - Takes PulseMotif stream
 * - Bins motifs into entropy classes
 * - Rewrites pulses to class-representative shapes
 */

#include "pulse_layers_api.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t count;
    uint8_t  motif_id;
} MotifStat;

/* simple helper */
static PulseSymbol mkpulse(double d, double p) {
    PulseSymbol s;
    s.duration_s = d;
    s.power_level = p;
    return s;
}

/* build stats for motif_id frequency */
static MotifStat *build_stats(const PulseMotif *motifs,
                              size_t count,
                              size_t *out_stats)
{
    MotifStat *stats = malloc(sizeof(MotifStat) * 256);
    for (int i = 0; i < 256; i++) {
        stats[i].count = 0;
        stats[i].motif_id = (uint8_t)i;
    }

    for (size_t i = 0; i < count; i++) {
        uint8_t id = motifs[i].motif_id;
        stats[id].count++;
    }

    *out_stats = 256;
    return stats;
}

/* map motif_id -> entropy class (0..7) based on frequency rank */
static void build_entropy_classes(const MotifStat *stats,
                                  size_t stat_count,
                                  uint8_t class_map[256])
{
    /* copy stats to temp for sorting */
    MotifStat *tmp = malloc(sizeof(MotifStat) * stat_count);
    memcpy(tmp, stats, sizeof(MotifStat) * stat_count);

    /* sort by descending count */
    for (size_t i = 0; i < stat_count; i++) {
        for (size_t j = i + 1; j < stat_count; j++) {
            if (tmp[j].count > tmp[i].count) {
                MotifStat t = tmp[i];
                tmp[i] = tmp[j];
                tmp[j] = t;
            }
        }
    }

    /* top motifs -> low entropy class, rare motifs -> high class */
    for (size_t rank = 0; rank < stat_count; rank++) {
        uint8_t id = tmp[rank].motif_id;
        uint8_t cls = (uint8_t)((rank * 8) / stat_count); /* 0..7 */
        class_map[id] = cls;
    }

    free(tmp);
}

/* representative pulse shapes for each entropy class */
static void class_rep_pulses(uint8_t cls, PulseSymbol out[8])
{
    double base_d = 2e-6 + cls * 1e-6;
    double base_p = 0.2 + cls * 0.1;

    for (int i = 0; i < 8; i++) {
        out[i] = mkpulse(base_d, base_p);
    }
}

/* PUBLIC API: entropy layer */
EntropyMotif *pulse_entropy_layer(const PulseMotif *motifs,
                                  size_t count,
                                  size_t *out_count)
{
    size_t stat_count = 0;
    MotifStat *stats = build_stats(motifs, count, &stat_count);

    uint8_t class_map[256];
    build_entropy_classes(stats, stat_count, class_map);
    free(stats);

    EntropyMotif *out = malloc(sizeof(EntropyMotif) * count);

    for (size_t i = 0; i < count; i++) {
        uint8_t id  = motifs[i].motif_id;
        uint8_t cls = class_map[id];

        EntropyMotif em;
        em.class_id = cls;
        em.motif_id = id;

        class_rep_pulses(cls, em.pulses);
        out[i] = em;
    }

    *out_count = count;
    return out;
}
