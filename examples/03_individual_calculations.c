/**
 * OpenEphemeris Example 3: Individual Natal Calculations
 *
 * Demonstrates separate Swiss Ephemeris-style calls for positions, houses,
 * aspects and Part of Fortune. No aggregate chart object is created.
 */

#include "openephemeris/oe.h"
#include <math.h>
#include <stdio.h>

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_time utc_jd;
    oe_position_result sun, moon;
    oe_house_result houses;
    oe_aspect_info aspect;
    oe_status status;
    double jd_ut, fortune;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Open ephemeris failed: %s\n", oe_status_string(status));
        return 1;
    }
    status = oe_utc_to_jd(1984, 6, 23, 5, 51, 0.0, NAN, &utc_jd);
    if (status != OE_OK) { oe_ephemeris_close(ephemeris); return 1; }
    jd_ut = utc_jd.jd_ut1;
    status = oe_position_at_jd(ephemeris, OE_SUN, jd_ut, &sun);
    if (status == OE_OK) status = oe_position_at_jd(ephemeris, OE_MOON, jd_ut, &moon);
    if (status != OE_OK) {
        fprintf(stderr, "Position failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    status = oe_houses_at_jd(jd_ut, 52.3594, 6.4665, OE_HOUSE_PLACIDUS, &houses);
    if (status != OE_OK) {
        fprintf(stderr, "Houses failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    status = oe_aspect_between_bodies_at_jd(ephemeris, OE_SUN, OE_MOON,
                                            jd_ut, 10.0, &aspect);
    if (status != OE_OK) {
        fprintf(stderr, "Aspect failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    fortune = oe_part_of_fortune(houses.ascendant_deg, sun.longitude_deg,
                                 moon.longitude_deg, 0);
    printf("OpenEphemeris - Individual Natal Calculations\n");
    printf("JD(UT): %.8f | location: 52.3594 N, 6.4665 E\n", jd_ut);
    printf("Sun: %.4f deg %s\n", sun.longitude_deg,
           oe_sign_name(oe_longitude_sign(sun.longitude_deg)));
    printf("Moon: %.4f deg %s\n", moon.longitude_deg,
           oe_sign_name(oe_longitude_sign(moon.longitude_deg)));
    printf("Ascendant: %.4f deg | MC: %.4f deg\n",
           houses.ascendant_deg, houses.midheaven_deg);
    printf("Sun/Moon: %s, orb %.4f deg\n",
           oe_aspect_name(aspect.type), aspect.orb_deg);
    printf("Part of Fortune: %.4f deg\n", fortune);
    oe_ephemeris_close(ephemeris);
    return 0;
}
