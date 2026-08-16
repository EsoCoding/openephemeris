#include "oe_internal.h"
#include "erfa.h"
#include <math.h>
#include <string.h>

static void ecliptic_to_equatorial(const oe_time *time,
                                   double longitude_deg,
                                   double latitude_deg,
                                   double *right_ascension_deg,
                                   double *declination_deg) {
    double longitude = longitude_deg * OE_D2R;
    double latitude = latitude_deg * OE_D2R;
    double dpsi, deps;
    double obliquity;
    double x, y, z;
    eraNut06a(2400000.5, time->jd_tt - 2400000.5, &dpsi, &deps);
    (void)dpsi;
    obliquity = eraObl06(2400000.5, time->jd_tt - 2400000.5) + deps;
    x = cos(latitude) * cos(longitude);
    y = cos(latitude) * sin(longitude) * cos(obliquity) - sin(latitude) * sin(obliquity);
    z = cos(latitude) * sin(longitude) * sin(obliquity) + sin(latitude) * cos(obliquity);
    *right_ascension_deg = oe_norm_deg(atan2(y, x) * OE_R2D);
    *declination_deg = atan2(z, hypot(x, y)) * OE_R2D;
}

oe_status oe_chart_from_utc(const oe_ephemeris *ephemeris,
                            int year, int month, int day,
                            int hour, int minute, double second,
                            double latitude_deg, double longitude_deg,
                            oe_chart_result *out) {
    oe_status status;
    size_t i;
    if (!ephemeris || !out) return OE_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    out->struct_size = sizeof(*out);
    out->abi_version = OE_ABI_VERSION;
    for (i = 0; i < OE_BODY_COUNT; ++i) out->house_positions[i] = NAN;
    status = oe_time_from_utc(year, month, day, hour, minute, second, NAN, &out->time);
    if (status != OE_OK) return status;
    status = oe_placidus_houses(&out->time, latitude_deg, longitude_deg, &out->houses);
    if (status != OE_OK) return status;
    for (i = 0; i < OE_BODY_COUNT; ++i) {
        double right_ascension, declination;
        status = oe_position(ephemeris, (oe_body)i, &out->time, &out->positions[i]);
        out->position_status[i] = status;
        if (status != OE_OK) {
            out->house_status[i] = status;
            continue;
        }
        ecliptic_to_equatorial(&out->time,
                               out->positions[i].longitude_deg,
                               out->positions[i].latitude_deg,
                               &right_ascension, &declination);
        status = oe_placidus_house_position(&out->time, latitude_deg,
                                            longitude_deg, right_ascension,
                                            declination,
                                            &out->house_positions[i]);
        out->house_status[i] = status;
    }
    return OE_OK;
}
