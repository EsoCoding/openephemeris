/**
 * OpenEphemeris Example 7: Sidereal and Vedic Calculations
 *
 * Demonstrates:
 * - Ayanamsa modes.
 * - Sidereal planetary positions.
 * - Nakshatra and pada numbering.
 * - Sidereal house cusps.
 */

#include "openephemeris/oe.h"
#include <math.h>
#include <stdio.h>

static void print_sidereal(const oe_ephemeris *ephemeris, oe_body body,
                           double jd_ut, oe_ayanamsa_mode mode,
                           const char *mode_name) {
    oe_sidereal_result result;
    oe_status status = oe_sidereal_position_at_jd(ephemeris, body, jd_ut, mode,
                                            0.0, &result);
    if (status != OE_OK) {
        printf("%-12s unavailable (%s)\n", mode_name, oe_status_string(status));
        return;
    }
    printf("%-12s ayanamsa %8.4f deg  sidereal %8.4f deg  nakshatra %2d  pada %d\n",
           mode_name, result.ayanamsa_deg, result.sidereal_longitude_deg,
           result.nakshatra + 1, result.pada);
}

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_house_result houses;
    oe_time utc_jd;
    oe_status status;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "Open ephemeris failed: %s\n", oe_status_string(status));
        fprintf(stderr, "Run: cmake --build build --target oe-data\n");
        return 1;
    }
    if (oe_utc_to_jd(1984, 6, 23, 5, 51, 0.0, NAN, &utc_jd) != OE_OK) {
        oe_ephemeris_close(ephemeris);
        return 1;
    }
    const double jd_ut = utc_jd.jd_ut1;

    printf("OpenEphemeris - Sidereal and Vedic Calculations\n");
    printf("Body: Sun | Date: 1984-06-23 05:51:00 UTC\n\n");
    print_sidereal(ephemeris, OE_SUN, jd_ut, OE_AYANAMSA_LAHIRI, "Lahiri");
    print_sidereal(ephemeris, OE_SUN, jd_ut, OE_AYANAMSA_RAMAN, "Raman");
    print_sidereal(ephemeris, OE_SUN, jd_ut, OE_AYANAMSA_KRISHNAMURTI, "KP");
    print_sidereal(ephemeris, OE_SUN, jd_ut, OE_AYANAMSA_TRUE_CITRA, "True Citra");

    status = oe_sidereal_houses_at_jd(jd_ut, 52.3594, 6.4665, OE_HOUSE_WHOLE_SIGN,
                                OE_AYANAMSA_LAHIRI, 0.0, &houses);
    if (status == OE_OK) {
        printf("\nLahiri sidereal Whole Sign houses:\n");
        printf("Ascendant: %.4f deg\n", houses.ascendant_deg);
        printf("House 1 cusp: %.4f deg\n", houses.cusps_deg[0]);
        printf("House 10 cusp: %.4f deg\n", houses.cusps_deg[9]);
    } else {
        printf("\nSidereal houses unavailable: %s\n", oe_status_string(status));
    }

    oe_ephemeris_close(ephemeris);
    return 0;
}
