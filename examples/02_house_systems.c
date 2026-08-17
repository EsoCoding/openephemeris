/**
 * OpenEphemeris Example 2: House Systems & Cusps
 * 
 * Demonstrates:
 * - Calculating Ascendant, Midheaven (MC), IC, and Descendant.
 * - Comparing different house systems (Placidus, Koch, Regiomontanus, Campanus, Whole Sign, etc.).
 * - Determining the exact house position of a point on the sky.
 */

#include "openephemeris/oe.h"
#include <stdio.h>
#include <math.h>

static void print_house_cusps(const char *system_name, const oe_house_result *h) {
    int i;
    printf("\n--- House System: %s ---\n", system_name);
    printf("Ascendant:   %7.3f°  (%s)\n", h->ascendant_deg, oe_sign_name(oe_longitude_sign(h->ascendant_deg)));
    printf("Midheaven:   %7.3f°  (%s)\n", h->midheaven_deg, oe_sign_name(oe_longitude_sign(h->midheaven_deg)));
    printf("ARMC (RAMC): %7.3f°\n", h->armc_deg);
    printf("Cusps:\n");
    for (i = 0; i < 12; ++i) {
        oe_zodiac_sign sign = oe_longitude_sign(h->cusps_deg[i]);
        double in_sign = oe_longitude_in_sign(h->cusps_deg[i]);
        printf("  House %2d: %7.3f°  (%02.0f° %-11s)\n",
               i + 1,
               h->cusps_deg[i],
               floor(in_sign),
               oe_sign_name(sign));
    }
}

int main(void) {
    oe_time time;
    oe_house_result houses;
    oe_status status;
    double lat = 52.3676; /* Amsterdam (52°22'N) */
    double lon = 4.9041;  /* Amsterdam (4°54'E) */

    /* Create time: 2000-01-01 12:00:00 UTC */
    status = oe_time_from_utc(2000, 1, 1, 12, 0, 0.0, NAN, &time);
    if (status != OE_OK) {
        fprintf(stderr, "Error converting time: %s\n", oe_status_string(status));
        return 1;
    }

    printf("========================================================================================\n");
    printf(" OpenEphemeris - House Systems Comparison\n");
    printf(" Location: Lat %+7.4f°, Lon %+7.4f° (Amsterdam)\n", lat, lon);
    printf(" Date/Time: 2000-01-01 12:00:00 UTC\n");
    printf("========================================================================================\n");

    /* 1. Placidus Houses (Default & Most popular) */
    if (oe_houses(&time, lat, lon, OE_HOUSE_PLACIDUS, &houses) == OE_OK) {
        print_house_cusps("Placidus ('P')", &houses);
    }

    /* 2. Koch Houses */
    if (oe_houses(&time, lat, lon, OE_HOUSE_KOCH, &houses) == OE_OK) {
        print_house_cusps("Koch ('K')", &houses);
    }

    /* 3. Regiomontanus Houses */
    if (oe_houses(&time, lat, lon, OE_HOUSE_REGIOMONTANUS, &houses) == OE_OK) {
        print_house_cusps("Regiomontanus ('R')", &houses);
    }

    /* 4. Whole Sign Houses */
    if (oe_houses(&time, lat, lon, OE_HOUSE_WHOLE_SIGN, &houses) == OE_OK) {
        print_house_cusps("Whole Sign ('W')", &houses);
    }

    /* 5. Porphyry Houses */
    if (oe_houses(&time, lat, lon, OE_HOUSE_PORPHYRY, &houses) == OE_OK) {
        print_house_cusps("Porphyry ('O')", &houses);
    }

    printf("\n========================================================================================\n");
    return 0;
}
