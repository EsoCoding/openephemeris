#include "openephemeris/oe.h"
#include <stdio.h>

int main(void) {
    const double jd_ut = 2445883.70208333;
    oe_ephemeris *ephemeris = NULL;
    oe_house_result houses;
    oe_status status;
    int body;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "OpenEphemeris data: %s\nRun: cmake --build build --target oe-data\n",
                oe_status_string(status));
        return 1;
    }
    status = oe_houses_at_jd(jd_ut, 52.3594, 6.4665,
                             OE_HOUSE_PLACIDUS, &houses);
    if (status != OE_OK) {
        fprintf(stderr, "Houses: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    printf("JD(UT) %.8f | Ascendant %10.6f | MC %10.6f\n\n",
           jd_ut, houses.ascendant_deg, houses.midheaven_deg);
    for (body = OE_SUN; body < OE_BODY_COUNT; ++body) {
        oe_position_result position;
        status = oe_position_at_jd(ephemeris, (oe_body)body, jd_ut, &position);
        if (status == OE_OK) {
            printf("%-16s %10.6f\n", oe_body_name((oe_body)body),
                   position.longitude_deg);
        } else {
            printf("%-16s unavailable (%s)\n", oe_body_name((oe_body)body),
                   oe_status_string(status));
        }
    }
    oe_ephemeris_close(ephemeris);
    return 0;
}
