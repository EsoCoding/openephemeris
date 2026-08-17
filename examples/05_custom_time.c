/**
 * OpenEphemeris Example 5: High-Precision Time Systems
 * 
 * Demonstrates:
 * - Converting Calendar UTC to Julian Dates (TT, UT1).
 * - Applying DUT1 (UT1 - UTC) and understanding Delta-T (TT - UT1).
 * - Direct time initialization with Julian Days (oe_time_from_jd).
 */

#include "openephemeris/oe.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    oe_time t1, t2;
    oe_status status;
    double delta_t_sec;

    /* 1. Create time from standard UTC calendar values */
    /* Example: J2000.0 Epoch: 2000-01-01 12:00:00 UTC */
    status = oe_time_from_utc(2000, 1, 1, 12, 0, 0.0, 0.0 /* dut1 */, &t1);
    if (status != OE_OK) {
        fprintf(stderr, "Time error: %s\n", oe_status_string(status));
        return 1;
    }

    delta_t_sec = (t1.jd_tt - t1.jd_ut1) * 86400.0;

    printf("========================================================================================\n");
    printf(" OpenEphemeris - High-Precision Time Conversions\n");
    printf("========================================================================================\n");
    printf(" Input UTC:  2000-01-01 12:00:00.000\n");
    printf("   Julian Date (TT)  : %16.8f\n", t1.jd_tt);
    printf("   Julian Date (UT1) : %16.8f\n", t1.jd_ut1);
    printf("   Delta-T (TT - UT1): %8.3f seconds\n", delta_t_sec);
    printf("   Quality Flags     : 0x%04X (Leap seconds known: %s)\n",
           t1.quality,
           (t1.quality & OE_TIME_LEAP_SECONDS_KNOWN) ? "Yes" : "No");

    /* 2. Direct time from raw Julian Day Numbers */
    status = oe_time_from_jd(2451545.00074287, 2451545.0, &t2);
    if (status == OE_OK) {
        printf("\n Direct JD initialization:\n");
        printf("   jd_tt = %.8f, jd_ut1 = %.8f\n", t2.jd_tt, t2.jd_ut1);
    }
    printf("========================================================================================\n");

    return 0;
}
