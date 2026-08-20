/**
 * OpenEphemeris Example 1: Planetary Positions
 * 
 * Demonstrates:
 * - Opening the default JPL ephemeris kernel.
 * - Calculating accurate planetary positions and speeds.
 * - Formatting coordinates as Zodiac signs (e.g. 15° Leo 24') and detecting retrograde motion.
 */

#include "openephemeris/oe.h"
#include <stdio.h>
#include <math.h>

static void print_formatted_position(const char *name, double lon_deg, double lat_deg, double speed_deg_day) {
    oe_zodiac_sign sign = oe_longitude_sign(lon_deg);
    double in_sign = oe_longitude_in_sign(lon_deg);
    int deg = (int)in_sign;
    int min = (int)((in_sign - deg) * 60.0);
    double sec = (in_sign - deg - min / 60.0) * 3600.0;
    const char *motion = (speed_deg_day < 0.0) ? " [Rx]" : "     ";

    printf("%-16s %2d° %-11s %02d'%04.1f\" %s  (lon: %7.3f°, lat: %+6.3f°, speed: %+6.3f°/d)\n",
           name,
           deg,
           oe_sign_name(sign),
           min,
           sec,
           motion,
           lon_deg,
           lat_deg,
           speed_deg_day);
}

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_status status;
    size_t i;

    /* 1. Open the ephemeris (uses default data path / data files) */
    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Could not open ephemeris: %s\n", oe_status_string(status));
        fprintf(stderr, "Ensure data kernels are built with: cmake --build build --target oe-data\n");
        return 1;
    }

    /* 2. Convert UTC once; all calculations below use the resulting JD(UT). */
    oe_time utc_jd;
    status = oe_utc_to_jd(2026, 8, 17, 12, 0, 0.0, NAN, &utc_jd);
    if (status != OE_OK) {
        fprintf(stderr, "UTC to JD failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    const double jd_ut = utc_jd.jd_ut1;

    printf("========================================================================================\n");
    printf(" OpenEphemeris - Planetary Positions (2026-08-17 12:00:00 UTC)\n");
    printf(" Julian Date (UT): %.6f\n", jd_ut);
    printf("========================================================================================\n");

    /* 3. Calculate position for each astrological body */
    for (i = 0; i < OE_BODY_COUNT; ++i) {
        oe_position_result pos;
        oe_body body = (oe_body)i;
        status = oe_position_at_jd(ephemeris, body, jd_ut, &pos);

        if (status == OE_OK) {
            print_formatted_position(oe_body_name(body),
                                     pos.longitude_deg,
                                     pos.latitude_deg,
                                     pos.longitude_speed_deg_per_day);
        } else {
            printf("%-16s [Coverage unavailable in active kernel: %s]\n",
                   oe_body_name(body), oe_status_string(status));
        }
    }

    printf("========================================================================================\n");

    /* 4. Always close the ephemeris kernel when finished */
    oe_ephemeris_close(ephemeris);
    return 0;
}
