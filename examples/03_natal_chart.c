/**
 * OpenEphemeris Example 3: Natal Chart & Element / Modality Balance
 * 
 * Demonstrates:
 * - Generating a full astrological chart in a single call with oe_chart_from_utc.
 * - Planetary placement in Zodiac Signs and Houses.
 * - Part of Fortune calculation (Pars Fortunae) for Day and Night charts.
 * - Element (Fire, Earth, Air, Water) and Modality (Cardinal, Fixed, Mutable) totals.
 */

#include "openephemeris/oe.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_chart_result chart;
    oe_status status;
    size_t i;
    int elements[4] = {0};   /* Fire, Earth, Air, Water */
    int modalities[3] = {0}; /* Cardinal, Fixed, Mutable */
    int is_night_chart;
    double part_of_fortune;

    /* 1. Open ephemeris */
    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Error opening ephemeris: %s\n", oe_status_string(status));
        return 1;
    }

    /* 2. Compute full chart for Amsterdam (1984-06-23 05:51:00 UTC) */
    status = oe_chart_from_utc(ephemeris,
                               1984, 6, 23, 5, 51, 0.0,
                               52.3594, 6.4665,
                               &chart);
    if (status != OE_OK) {
        fprintf(stderr, "Chart calculation failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }

    printf("========================================================================================\n");
    printf(" OpenEphemeris - Natal Chart Report\n");
    printf(" Birth: 1984-06-23 05:51:00 UTC | Location: 52.3594°N, 6.4665°E\n");
    printf("========================================================================================\n");
    printf(" Angles:\n");
    printf("   Ascendant:  %6.2f°  %s %s\n",
           chart.houses.ascendant_deg,
           oe_sign_symbol(oe_longitude_sign(chart.houses.ascendant_deg)),
           oe_sign_name(oe_longitude_sign(chart.houses.ascendant_deg)));
    printf("   Midheaven:  %6.2f°  %s %s\n\n",
           chart.houses.midheaven_deg,
           oe_sign_symbol(oe_longitude_sign(chart.houses.midheaven_deg)),
           oe_sign_name(oe_longitude_sign(chart.houses.midheaven_deg)));

    printf(" Planetary Placements:\n");
    printf(" %-15s %-12s %-6s %-10s %-8s\n", "Body", "Longitude", "Sign", "House", "Motion");
    printf(" --------------------------------------------------------------------------------------\n");

    for (i = 0; i < OE_BODY_COUNT; ++i) {
        if (chart.position_status[i] == OE_OK) {
            double lon = chart.positions[i].longitude_deg;
            double in_sign = oe_longitude_in_sign(lon);
            oe_zodiac_sign sign = oe_longitude_sign(lon);
            const char *motion = (chart.positions[i].longitude_speed_deg_per_day < 0.0) ? "Retrograde" : "Direct";

            printf(" %-15s %2.0f° %02.0f' %-7s %s     House %4.1f   %s\n",
                   oe_body_name((oe_body)i),
                   floor(in_sign),
                   floor(fmod(in_sign * 60.0, 60.0)),
                   oe_sign_name(sign),
                   oe_sign_symbol(sign),
                   chart.house_positions[i],
                   motion);

            /* Count elements and modalities for main 10 bodies */
            if (i <= OE_PLUTO) {
                elements[oe_sign_element(sign)]++;
                modalities[oe_sign_modality(sign)]++;
            }
        }
    }

    /* 3. Check if Day Chart or Night Chart (Sun in houses 7..12 = Day, houses 1..6 = Night) */
    is_night_chart = (chart.house_positions[OE_SUN] < 7.0);
    part_of_fortune = oe_part_of_fortune(chart.houses.ascendant_deg,
                                         chart.positions[OE_SUN].longitude_deg,
                                         chart.positions[OE_MOON].longitude_deg,
                                         is_night_chart);

    printf("\n Special Points:\n");
    printf("   Part of Fortune: %6.2f° (%s %s) [%s Chart]\n",
           part_of_fortune,
           oe_sign_symbol(oe_longitude_sign(part_of_fortune)),
           oe_sign_name(oe_longitude_sign(part_of_fortune)),
           is_night_chart ? "Night" : "Day");

    printf("\n Element & Modality Balance (10 Primary Bodies):\n");
    printf("   Fire:  %d | Earth: %d | Air: %d | Water: %d\n",
           elements[OE_ELEMENT_FIRE], elements[OE_ELEMENT_EARTH],
           elements[OE_ELEMENT_AIR], elements[OE_ELEMENT_WATER]);
    printf("   Cardinal: %d | Fixed: %d | Mutable: %d\n",
           modalities[OE_MODALITY_CARDINAL],
           modalities[OE_MODALITY_FIXED],
           modalities[OE_MODALITY_MUTABLE]);
    printf("========================================================================================\n");

    oe_ephemeris_close(ephemeris);
    return 0;
}
