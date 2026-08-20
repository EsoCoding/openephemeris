/**
 * OpenEphemeris Example 4: Aspect Matrix
 *
 * Demonstrates direct body-to-body aspect calculations at one UT Julian date.
 */

#include "openephemeris/oe.h"
#include <stdio.h>

int main(void) {
    const double jd_ut = 2451545.0;
    oe_ephemeris *ephemeris = NULL;
    oe_status status;
    int body1, body2;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Open ephemeris failed: %s\n", oe_status_string(status));
        return 1;
    }
    printf("OpenEphemeris - Aspect Matrix\n");
    printf("JD(UT): %.8f\n\n", jd_ut);
    for (body1 = OE_SUN; body1 <= OE_PLUTO; ++body1) {
        for (body2 = body1 + 1; body2 <= OE_PLUTO; ++body2) {
            oe_aspect_info aspect;
            status = oe_aspect_between_bodies_at_jd(
                ephemeris, (oe_body)body1, (oe_body)body2, jd_ut, 6.0, &aspect);
            if (status == OE_OK && aspect.type != OE_ASPECT_NONE) {
                printf("%-8s %-8s %-14s orb %.3f deg\n",
                       oe_body_name((oe_body)body1),
                       oe_body_name((oe_body)body2),
                       oe_aspect_name(aspect.type), aspect.orb_deg);
            }
        }
    }
    oe_ephemeris_close(ephemeris);
    return 0;
}
