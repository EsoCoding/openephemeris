#include "oe_internal.h"
#include "erfa.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

/* Compact bright-star catalogue. Coordinates are J2000 ICRS and motions are
 * sufficient for astrological use; the catalogue is intentionally data-only. */
static const oe_fixed_star stars[] = {
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Aldebaran", "Tau", 0.85, 68.980163, 16.509302, 62.78, -188.94},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Antares", "Sco", 0.96, 247.351915, -26.432002, -23.21, -10.18},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Regulus", "Leo", 1.40, 152.092962, 11.967208, -248.73, 5.59},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Spica", "Vir", 0.98, 201.298247, -11.161322, -42.50, -31.73},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Sirius", "CMa", -1.46, 101.287155, -16.716116, -546.01, -1223.07},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Pollux", "Gem", 1.14, 116.328958, 28.026199, -626.55, -45.80},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Fomalhaut", "PsA", 1.16, 344.412750, -29.622236, 328.95, -164.67},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Procyon", "CMi", 0.34, 114.825493, 5.224993, -714.59, -1036.80},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Polaris", "UMi", 1.98, 37.954560, 89.264108, 44.22, -11.74},
    {sizeof(oe_fixed_star), OE_ABI_VERSION, "Vega", "Lyr", 0.03, 279.234735, 38.783689, 200.94, 286.23}
};

static int same_name(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        ++a; ++b;
    }
    return *a == *b;
}

size_t oe_fixed_star_count(void) { return sizeof(stars) / sizeof(stars[0]); }
const oe_fixed_star *oe_fixed_star_at(size_t i) {
    return i < oe_fixed_star_count() ? &stars[i] : NULL;
}
const oe_fixed_star *oe_fixed_star_find(const char *name) {
    size_t i;
    if (!name) return NULL;
    for (i = 0; i < oe_fixed_star_count(); ++i)
        if (same_name(name, stars[i].name)) return &stars[i];
    /* Common Bayer/traditional aliases used by astrology applications. */
    if (same_name(name, "Alpha Tauri")) return &stars[0];
    if (same_name(name, "Alpha Scorpii")) return &stars[1];
    if (same_name(name, "Alpha Leonis")) return &stars[2];
    if (same_name(name, "Alpha Virginis")) return &stars[3];
    if (same_name(name, "Alpha Canis Majoris")) return &stars[4];
    if (same_name(name, "Alpha Piscis Austrini")) return &stars[5];
    return NULL;
}

oe_status oe_fixed_star_position(const oe_fixed_star *star, const oe_time *time,
                                 oe_position_result *out) {
    double ra, dec, lon, lat;
    double years;
    if (!star || !time || !out || time->struct_size < sizeof(*time) ||
        time->abi_version != OE_ABI_VERSION) return OE_ERR_INVALID_ARGUMENT;
    years = (time->jd_tt - OE_J2000) / 365.25;
    ra = star->ra_j2000_deg * OE_D2R +
         star->proper_motion_ra_mas_year * OE_D2R / 3600000.0 * years;
    dec = star->dec_j2000_deg * OE_D2R +
          star->proper_motion_dec_mas_year * OE_D2R / 3600000.0 * years;
    eraEqec06(2400000.5, time->jd_tt - 2400000.5, ra, dec, &lon, &lat);
    memset(out, 0, sizeof(*out));
    out->struct_size = sizeof(*out); out->abi_version = OE_ABI_VERSION;
    out->longitude_deg = oe_norm_deg(lon * OE_R2D);
    out->latitude_deg = lat * OE_R2D;
    out->distance_au = 1.0e9;
    return OE_OK;
}

double oe_ayanamsa(const oe_time *time, oe_ayanamsa_mode mode,
                   double user_value_deg) {
    double t, base;
    if (!time || !isfinite(time->jd_tt)) return NAN;
    t = (time->jd_tt - OE_J2000) / 36525.0;
    if (mode == OE_AYANAMSA_USER) return user_value_deg;
    /* Linearized Swiss Ephemeris-compatible starting points, with the common
     * 50.29 arcsec/year precession rate. */
    switch (mode) {
    case OE_AYANAMSA_LAHIRI: base = 23.85675; break;
    case OE_AYANAMSA_RAMAN: base = 22.46000; break;
    case OE_AYANAMSA_KRISHNAMURTI: base = 23.99900; break;
    case OE_AYANAMSA_YUKTESHWAR: base = 22.22600; break;
    case OE_AYANAMSA_TRUE_CITRA: base = 23.87000; break;
    case OE_AYANAMSA_TRUE_REVATI: base = 23.30000; break;
    case OE_AYANAMSA_TRUE_PUSHYA: base = 23.95000; break;
    case OE_AYANAMSA_FAGAN_BRADLEY:
    default: base = 24.04200; break;
    }
    return base + 0.5029 * t;
}

oe_status oe_sidereal_position(const oe_ephemeris *e, oe_body body,
                               const oe_time *time, oe_ayanamsa_mode mode,
                               double user_value_deg, oe_sidereal_result *out) {
    oe_position_result p;
    double a;
    if (!e || !time || !out) return OE_ERR_INVALID_ARGUMENT;
    if (oe_position(e, body, time, &p) != OE_OK) return OE_ERR_NO_COVERAGE;
    a = oe_ayanamsa(time, mode, user_value_deg);
    if (!isfinite(a)) return OE_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->struct_size = sizeof(*out);
    out->abi_version = OE_ABI_VERSION; out->ayanamsa_deg = a;
    out->tropical_longitude_deg = p.longitude_deg;
    out->sidereal_longitude_deg = oe_norm_deg(p.longitude_deg - a);
    out->nakshatra = (int)floor(out->sidereal_longitude_deg / (360.0 / 27.0));
    out->pada = (int)floor(fmod(out->sidereal_longitude_deg, 360.0 / 27.0) /
                            (360.0 / 108.0)) + 1;
    return OE_OK;
}

oe_status oe_sidereal_houses(const oe_time *time, double lat, double lon,
                             int system, oe_ayanamsa_mode mode,
                             double user_value_deg, oe_house_result *out) {
    oe_status s; double a; int i;
    if (!time || !out) return OE_ERR_INVALID_ARGUMENT;
    s = oe_houses(time, lat, lon, system, out); if (s != OE_OK) return s;
    a = oe_ayanamsa(time, mode, user_value_deg);
    out->ascendant_deg = oe_norm_deg(out->ascendant_deg - a);
    out->midheaven_deg = oe_norm_deg(out->midheaven_deg - a);
    out->armc_deg = oe_norm_deg(out->armc_deg - a);
    for (i = 0; i < 12; ++i) out->cusps_deg[i] = oe_norm_deg(out->cusps_deg[i] - a);
    return OE_OK;
}

static oe_status search_body(const oe_ephemeris *e, oe_body body, double target,
                             const oe_time *start, int direction, double days,
                             oe_search_result *out) {
    double jd0, jd, prev, cur, step, end, x, y, mid, fmid;
    oe_time t; oe_position_result p; int i, n;
    if (!e || !start || !out || (direction != 1 && direction != -1) ||
        !isfinite(days) || days <= 0.0) return OE_ERR_INVALID_ARGUMENT;
    step = direction * 0.25; jd0 = start->jd_tt; end = jd0 + direction * days;
    t = *start; if (oe_position(e, body, &t, &p) != OE_OK) return OE_ERR_NO_COVERAGE;
    prev = remainder(p.longitude_deg - target, 360.0); n = (int)ceil(days / 0.25);
    for (i = 1; i <= n; ++i) {
        jd = jd0 + direction * fmin(days, i * 0.25); t.jd_tt = jd;
        if (oe_position(e, body, &t, &p) != OE_OK) return OE_ERR_NO_COVERAGE;
        cur = remainder(p.longitude_deg - target, 360.0);
        if ((direction > 0 && cur >= 0.0 && prev <= 0.0) ||
            (direction < 0 && cur <= 0.0 && prev >= 0.0) || fabs(cur) < 1e-7) {
            x = jd - step; y = jd; if (direction < 0) { x = jd; y = jd - step; }
            for (int k = 0; k < 45; ++k) {
                mid = (x + y) * 0.5; t.jd_tt = mid; oe_position(e, body, &t, &p);
                fmid = remainder(p.longitude_deg - target, 360.0);
                if ((direction > 0 && fmid > 0.0) || (direction < 0 && fmid < 0.0)) y = mid;
                else x = mid;
            }
            t.jd_tt = (x + y) * 0.5; oe_position(e, body, &t, &p);
            memset(out, 0, sizeof(*out)); out->struct_size = sizeof(*out);
            out->abi_version = OE_ABI_VERSION; out->time = t; out->position = p;
            out->target_longitude_deg = oe_norm_deg(target);
            out->residual_deg = remainder(p.longitude_deg - target, 360.0);
            return OE_OK;
        }
        prev = cur;
    }
    (void)end; return OE_ERR_NO_COVERAGE;
}

oe_status oe_transit_search(const oe_ephemeris *e, oe_body body, double target,
                            const oe_time *start, int direction, double days,
                            oe_search_result *out) {
    return search_body(e, body, target, start, direction, days, out);
}
oe_status oe_return_search(const oe_ephemeris *e, oe_body body, double natal,
                           const oe_time *start, int direction, double days,
                           oe_search_result *out) {
    return search_body(e, body, natal, start, direction, days, out);
}

oe_status oe_eclipse_search(const oe_ephemeris *e, oe_eclipse_type type,
                            const oe_time *start, int direction, double days,
                            oe_eclipse_result *out) {
    oe_search_result phase; oe_time t; oe_position_result sun, moon;
    double target = type == OE_ECLIPSE_SOLAR ? 0.0 : 180.0;
    double jd0, jd, prev, cur, step, x, y, mid, fmid;
    int i, n;
    if (!e || !start || !out) return OE_ERR_INVALID_ARGUMENT;
    if ((direction != 1 && direction != -1) || days <= 0.0) return OE_ERR_INVALID_ARGUMENT;
    jd0 = start->jd_tt; step = direction * 0.25; n = (int)ceil(days / 0.25);
    t = *start; if (oe_position(e, OE_SUN, &t, &sun) != OE_OK ||
        oe_position(e, OE_MOON, &t, &moon) != OE_OK) return OE_ERR_NO_COVERAGE;
    prev = remainder(moon.longitude_deg - sun.longitude_deg - target, 360.0);
    for (i = 1; i <= n; ++i) {
        jd = jd0 + direction * fmin(days, i * 0.25); t.jd_tt = jd;
        if (oe_position(e, OE_SUN, &t, &sun) != OE_OK ||
            oe_position(e, OE_MOON, &t, &moon) != OE_OK) return OE_ERR_NO_COVERAGE;
        cur = remainder(moon.longitude_deg - sun.longitude_deg - target, 360.0);
        if ((direction > 0 && cur >= 0.0 && prev <= 0.0) ||
            (direction < 0 && cur <= 0.0 && prev >= 0.0) || fabs(cur) < 1e-7) {
            x = jd - step; y = jd; if (direction < 0) { x = jd; y = jd - step; }
            for (int k = 0; k < 45; ++k) {
                mid = (x + y) * 0.5; t.jd_tt = mid;
                oe_position(e, OE_SUN, &t, &sun); oe_position(e, OE_MOON, &t, &moon);
                fmid = remainder(moon.longitude_deg - sun.longitude_deg - target, 360.0);
                if ((direction > 0 && fmid > 0.0) || (direction < 0 && fmid < 0.0)) y = mid;
                else x = mid;
            }
            t.jd_tt = (x + y) * 0.5;
            if (oe_position(e, OE_SUN, &t, &sun) != OE_OK ||
                oe_position(e, OE_MOON, &t, &moon) != OE_OK) return OE_ERR_NO_COVERAGE;
            memset(&phase, 0, sizeof(phase)); phase.time = t; phase.position = moon;
            break;
        }
        prev = cur;
        if (i == n) return OE_ERR_NO_COVERAGE;
    }
    if (fabs(moon.latitude_deg) > 1.6) return OE_ERR_NO_COVERAGE;
    memset(out, 0, sizeof(*out)); out->struct_size = sizeof(*out);
    out->abi_version = OE_ABI_VERSION; out->type = type; out->maximum = t;
    out->longitude_deg = sun.longitude_deg; out->latitude_deg = moon.latitude_deg;
    out->magnitude = fmax(0.0, 1.0 - fabs(moon.latitude_deg) / 1.6);
    out->totality = out->magnitude > 0.95;
    return OE_OK;
}
