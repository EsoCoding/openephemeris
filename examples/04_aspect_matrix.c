/**
 * OpenEphemeris Example 4: Astrological Aspects & Aspect Grid
 * 
 * Demonstrates:
 * - Calculating all aspects (Conjunction, Trine, Square, Sextile, Opposition, Quincunx, etc.).
 * - Inspecting orbs and detecting Applying vs. Separating aspect dynamics.
 * - Aspect detection with Angles (Ascendant, Midheaven).
 */

#include "openephemeris/oe.h"
#include <stdio.h>

static const char *get_body_label(int body_id) {
    if (body_id == -1) return "Ascendant";
    if (body_id == -2) return "Midheaven";
    if (body_id >= 0 && body_id < (int)OE_BODY_COUNT) {
        return oe_body_name((oe_body)body_id);
    }
    return "Unknown";
}

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_chart_result chart;
    oe_aspect_info aspects[128];
    oe_status status;
    size_t count, i;

    /* 1. Open ephemeris and compute chart */
    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Ephemeris error: %s\n", oe_status_string(status));
        return 1;
    }

    status = oe_chart_from_utc(ephemeris,
                               2000, 1, 1, 12, 0, 0.0,
                               52.3676, 4.9041, /* Amsterdam */
                               &chart);
    if (status != OE_OK) {
        fprintf(stderr, "Chart error: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }

    /* 2. Compute all astrological aspects present in this chart */
    count = oe_chart_aspects(&chart, aspects, 128);

    printf("========================================================================================\n");
    printf(" OpenEphemeris - Astrological Aspect Grid\n");
    printf(" Found %zu active aspects:\n", count);
    printf("========================================================================================\n");
    printf(" %-15s %-4s %-15s %-16s %-8s %-12s\n",
           "Body 1", "Sym", "Body 2", "Aspect Type", "Orb", "Dynamics");
    printf(" --------------------------------------------------------------------------------------\n");

    for (i = 0; i < count; ++i) {
        const char *b1 = get_body_label(aspects[i].body1);
        const char *b2 = get_body_label(aspects[i].body2);
        const char *asp_sym = oe_aspect_symbol(aspects[i].type);
        const char *asp_name = oe_aspect_name(aspects[i].type);
        const char *dyn = (aspects[i].flags & OE_ASPECT_EXACT) ? "Exact" :
                          (aspects[i].flags & OE_ASPECT_APPLYING) ? "Applying" : "Separating";

        printf(" %-15s  %s   %-15s %-16s %5.2f°   %-12s\n",
               b1, asp_sym, b2, asp_name, aspects[i].orb_deg, dyn);
    }

    printf("========================================================================================\n");

    oe_ephemeris_close(ephemeris);
    return 0;
}
