/**
 * OpenEphemeris Example 5: High-Precision Time Systems
 * 
 * Demonstrates:
 * - Converting Calendar UTC to Julian Dates (UT1 and derived TT).
 * - Applying DUT1 (UT1 - UTC) and understanding Delta-T (TT - UT1).
 * - Using the resulting jd_ut1 as the calculation input.
 */

#include "openephemeris/oe.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    oe_time time;
    oe_status status;
    double delta_t_sec;

    /* 1. Create time from standard UTC calendar values */
    /* Example: J2000.0 Epoch: 2000-01-01 12:00:00 UTC */
    status = oe_utc_to_jd(2000, 1, 1, 12, 0, 0.0, 0.0 /* dut1 */, &time);
    if (status != OE_OK) {
        fprintf(stderr, "Time error: %s\n", oe_status_string(status));
        return 1;
    }

    delta_t_sec = (time.jd_tt - time.jd_ut1) * 86400.0;

    printf("========================================================================================\n");
    printf(" OpenEphemeris - High-Precision Time Conversions\n");
    printf("========================================================================================\n");
    printf(" Input UTC:  2000-01-01 12:00:00.000\n");
    printf("   Julian Date (UT1) : %16.8f\n", time.jd_ut1);
    printf("   Julian Date (TT)  : %16.8f\n", time.jd_tt);
    printf("   Calculation input : jd_ut = %.8f\n", time.jd_ut1);
    printf("   Delta-T (TT - UT1): %8.3f seconds\n", delta_t_sec);
    printf("   Quality Flags     : 0x%04X (Leap seconds known: %s)\n",
           time.quality,
           (time.quality & OE_TIME_LEAP_SECONDS_KNOWN) ? "Yes" : "No");
    printf("========================================================================================\n");

    return 0;
}
