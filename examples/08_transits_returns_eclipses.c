/**
 * OpenEphemeris Example 8: Transits, Returns, and Eclipses
 *
 * Demonstrates:
 * - Searching a planetary transit to a requested longitude.
 * - Solar and lunar return searches using natal longitudes.
 * - Searching the next solar and lunar eclipse.
 *
 * Search bounds are expressed in days and direction is +1 (forward) or -1.
 */

#include "openephemeris/oe.h"
#include <math.h>
#include <stdio.h>

static void print_search(const char *label, const oe_search_result *result) {
    printf("%-18s JD(TT) %.6f  longitude %.6f deg  residual %+g deg\n",
           label, result->time.jd_tt, result->position.longitude_deg,
           result->residual_deg);
}

static void print_eclipse(const char *label, const oe_eclipse_result *result) {
    printf("%-18s JD(TT) %.6f  longitude %.4f deg  latitude %+ .4f deg  magnitude %.3f\n",
           label, result->maximum.jd_tt, result->longitude_deg,
           result->latitude_deg, result->magnitude);
}

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_position_result natal_sun, natal_moon;
    oe_position_result current_mars;
    oe_search_result result;
    oe_eclipse_result eclipse;
    oe_time start_jd, natal_jd;
    oe_status status;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Open ephemeris failed: %s\n", oe_status_string(status));
        fprintf(stderr, "Run: cmake --build build --target oe-data\n");
        return 1;
    }
    if (oe_utc_to_jd(2026, 8, 20, 12, 0, 0.0, NAN, &start_jd) != OE_OK ||
        oe_utc_to_jd(1984, 6, 23, 5, 51, 0.0, NAN, &natal_jd) != OE_OK) {
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    const double start_jd_ut = start_jd.jd_ut1;
    const double natal_jd_ut = natal_jd.jd_ut1;
    if (oe_position_at_jd(ephemeris, OE_SUN, natal_jd_ut, &natal_sun) != OE_OK ||
        oe_position_at_jd(ephemeris, OE_MOON, natal_jd_ut, &natal_moon) != OE_OK ||
        oe_position_at_jd(ephemeris, OE_MARS, start_jd_ut, &current_mars) != OE_OK) {
        fprintf(stderr, "Could not prepare event inputs\n");
        oe_ephemeris_close(ephemeris);
        return 1;
    }

    printf("OpenEphemeris - Transits, Returns, and Eclipses\n");
    printf("Search start: 2026-08-20 12:00:00 UTC\n\n");

    {
        double target = current_mars.longitude_deg +
                        (current_mars.longitude_speed_deg_per_day < 0.0 ? -1.0 : 1.0);
        if (target >= 360.0) target -= 360.0;
        if (target < 0.0) target += 360.0;
        status = oe_transit_search(ephemeris, OE_MARS, target, start_jd_ut,
                                   1, 30.0, &result);
        if (status == OE_OK) print_search("Mars transit +1 deg", &result);
        else printf("Mars transit: %s\n", oe_status_string(status));
    }

    status = oe_return_search(ephemeris, OE_SUN, natal_sun.longitude_deg,
                              start_jd_ut, 1, 370.0, &result);
    if (status == OE_OK) print_search("Solar return", &result);
    else printf("Solar return: %s\n", oe_status_string(status));

    status = oe_return_search(ephemeris, OE_MOON, natal_moon.longitude_deg,
                              start_jd_ut, 1, 35.0, &result);
    if (status == OE_OK) print_search("Lunar return", &result);
    else printf("Lunar return: %s\n", oe_status_string(status));

    status = oe_eclipse_search(ephemeris, OE_ECLIPSE_SOLAR, start_jd_ut,
                               1,
                               400.0, &eclipse);
    if (status == OE_OK) print_eclipse("Solar eclipse", &eclipse);
    else printf("Solar eclipse: %s\n", oe_status_string(status));

    status = oe_eclipse_search(ephemeris, OE_ECLIPSE_LUNAR, start_jd_ut,
                               1,
                               400.0, &eclipse);
    if (status == OE_OK) print_eclipse("Lunar eclipse", &eclipse);
    else printf("Lunar eclipse: %s\n", oe_status_string(status));

    oe_ephemeris_close(ephemeris);
    return 0;
}
