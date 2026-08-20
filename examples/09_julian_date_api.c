/**
 * OpenEphemeris Example 9: Swiss Ephemeris-style Julian Date workflow
 *
 * One UT Julian date is passed to every calculation. Houses additionally use
 * location, while TT is derived internally for ephemeris positions.
 */

#include "openephemeris/oe.h"
#include <stdio.h>

int main(void) {
    const double jd_ut = 2451545.0;
    oe_ephemeris *ephemeris = NULL;
    oe_position_result sun;
    oe_house_result houses;
    oe_aspect_info aspect;
    oe_status status;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Open ephemeris failed: %s\n", oe_status_string(status));
        return 1;
    }
    status = oe_position_at_jd(ephemeris, OE_SUN, jd_ut, &sun);
    if (status != OE_OK) {
        fprintf(stderr, "Position failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    status = oe_houses_at_jd(jd_ut, 52.3676, 4.9041,
                             OE_HOUSE_WHOLE_SIGN, &houses);
    if (status != OE_OK) {
        fprintf(stderr, "Houses failed: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    status = oe_aspect_between_bodies_at_jd(ephemeris, OE_SUN, OE_MOON,
                                            jd_ut, 10.0, &aspect);
    printf("OpenEphemeris - Julian Date workflow\n");
    printf("jd_ut %.8f\n", jd_ut);
    printf("Sun: %.8f deg tropical longitude\n", sun.longitude_deg);
    printf("Whole Sign Ascendant: %.8f deg\n", houses.ascendant_deg);
    if (status == OE_OK) {
        printf("Sun/Moon: %s, orb %.4f deg\n",
               oe_aspect_name(aspect.type), aspect.orb_deg);
    }
    oe_ephemeris_close(ephemeris);
    return 0;
}
