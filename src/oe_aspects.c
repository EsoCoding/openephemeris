#include "openephemeris/oe.h"
#include "oe_internal.h"
#include <math.h>
#include <string.h>

static const char *const kSignNames[12] = {
    "Aries", "Taurus", "Gemini", "Cancer", "Leo", "Virgo",
    "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces"
};

static const char *const kSignSymbols[12] = {
    "♈", "♉", "♊", "♋", "♌", "♍",
    "♎", "♏", "♐", "♑", "♒", "♓"
};

static const oe_element kSignElements[12] = {
    OE_ELEMENT_FIRE,  OE_ELEMENT_EARTH, OE_ELEMENT_AIR,   OE_ELEMENT_WATER,
    OE_ELEMENT_FIRE,  OE_ELEMENT_EARTH, OE_ELEMENT_AIR,   OE_ELEMENT_WATER,
    OE_ELEMENT_FIRE,  OE_ELEMENT_EARTH, OE_ELEMENT_AIR,   OE_ELEMENT_WATER
};

static const oe_modality kSignModalities[12] = {
    OE_MODALITY_CARDINAL, OE_MODALITY_FIXED, OE_MODALITY_MUTABLE,
    OE_MODALITY_CARDINAL, OE_MODALITY_FIXED, OE_MODALITY_MUTABLE,
    OE_MODALITY_CARDINAL, OE_MODALITY_FIXED, OE_MODALITY_MUTABLE,
    OE_MODALITY_CARDINAL, OE_MODALITY_FIXED, OE_MODALITY_MUTABLE
};

typedef struct aspect_def {
    oe_aspect_type type;
    double angle;
    double default_major_orb;
    double default_minor_orb;
    const char *name;
    const char *symbol;
} aspect_def;

static const aspect_def kAspectDefs[] = {
    { OE_ASPECT_CONJUNCTION,    0.0,   10.0, 8.0, "Conjunction",    "☌" },
    { OE_ASPECT_OPPOSITION,     180.0, 10.0, 8.0, "Opposition",     "☍" },
    { OE_ASPECT_TRINE,          120.0,  8.0, 6.0, "Trine",          "△" },
    { OE_ASPECT_SQUARE,         90.0,   8.0, 6.0, "Square",         "□" },
    { OE_ASPECT_SEXTILE,        60.0,   6.0, 4.0, "Sextile",        "⚹" },
    { OE_ASPECT_QUINCUNX,       150.0,  3.0, 2.5, "Quincunx",       "⚻" },
    { OE_ASPECT_SEMISQUARE,     45.0,   2.5, 2.0, "Semisquare",     "∠" },
    { OE_ASPECT_SESQUIQUADRATE, 135.0,  2.5, 2.0, "Sesquiquadrate", "⚼" },
    { OE_ASPECT_SEMISEXTILE,    30.0,   2.0, 1.5, "Semisextile",    "⚺" },
    { OE_ASPECT_QUINTILE,       72.0,   2.0, 1.5, "Quintile",       "Q" },
    { OE_ASPECT_BIQUINTILE,     144.0,  2.0, 1.5, "Biquintile",     "bQ"}
};

#define NUM_ASPECT_DEFS (sizeof(kAspectDefs) / sizeof(kAspectDefs[0]))

const char *oe_sign_name(oe_zodiac_sign sign) {
    if ((int)sign < 0 || (int)sign >= 12) return "";
    return kSignNames[(int)sign];
}

const char *oe_sign_symbol(oe_zodiac_sign sign) {
    if ((int)sign < 0 || (int)sign >= 12) return "";
    return kSignSymbols[(int)sign];
}

oe_zodiac_sign oe_longitude_sign(double longitude_deg) {
    double lon = oe_norm_deg(longitude_deg);
    int sign = (int)(lon / 30.0);
    if (sign < 0) sign = 0;
    if (sign > 11) sign = 11;
    return (oe_zodiac_sign)sign;
}

double oe_longitude_in_sign(double longitude_deg) {
    double lon = oe_norm_deg(longitude_deg);
    return fmod(lon, 30.0);
}

oe_element oe_sign_element(oe_zodiac_sign sign) {
    if ((int)sign < 0 || (int)sign >= 12) return OE_ELEMENT_FIRE;
    return kSignElements[(int)sign];
}

oe_modality oe_sign_modality(oe_zodiac_sign sign) {
    if ((int)sign < 0 || (int)sign >= 12) return OE_MODALITY_CARDINAL;
    return kSignModalities[(int)sign];
}

const char *oe_aspect_name(oe_aspect_type type) {
    size_t i;
    if (type == OE_ASPECT_PARALLEL) return "Parallel";
    if (type == OE_ASPECT_CONTRAPARALLEL) return "Contra-Parallel";
    for (i = 0; i < NUM_ASPECT_DEFS; i++) {
        if (kAspectDefs[i].type == type) return kAspectDefs[i].name;
    }
    return "None";
}

const char *oe_aspect_symbol(oe_aspect_type type) {
    size_t i;
    if (type == OE_ASPECT_PARALLEL) return "||";
    if (type == OE_ASPECT_CONTRAPARALLEL) return "∦";
    for (i = 0; i < NUM_ASPECT_DEFS; i++) {
        if (kAspectDefs[i].type == type) return kAspectDefs[i].symbol;
    }
    return "";
}

double oe_aspect_angle(oe_aspect_type type) {
    size_t i;
    for (i = 0; i < NUM_ASPECT_DEFS; i++) {
        if (kAspectDefs[i].type == type) return kAspectDefs[i].angle;
    }
    return 0.0;
}

double oe_aspect_default_orb(oe_aspect_type type, int body1, int body2) {
    size_t i;
    int is_luminary = (body1 == OE_SUN || body1 == OE_MOON || body2 == OE_SUN || body2 == OE_MOON);
    if (type == OE_ASPECT_PARALLEL || type == OE_ASPECT_CONTRAPARALLEL) {
        return is_luminary ? 1.5 : 1.0;
    }
    for (i = 0; i < NUM_ASPECT_DEFS; i++) {
        if (kAspectDefs[i].type == type) {
            return is_luminary ? kAspectDefs[i].default_major_orb : kAspectDefs[i].default_minor_orb;
        }
    }
    return 6.0;
}

oe_status oe_aspect_calculate(double lon1, double speed1,
                              double lon2, double speed2,
                              double max_orb_override,
                              oe_aspect_info *out) {
    double diff, best_orb = 999.0;
    size_t i, best_idx = (size_t)-1;
    double max_orb;
    double rel_speed, diff_rate;

    if (!out || !isfinite(lon1) || !isfinite(lon2)) return OE_ERR_INVALID_ARGUMENT;

    memset(out, 0, sizeof(*out));
    diff = fabs(fmod(lon1 - lon2 + 540.0, 360.0) - 180.0);
    out->actual_diff_deg = diff;

    for (i = 0; i < NUM_ASPECT_DEFS; i++) {
        double target = kAspectDefs[i].angle;
        double orb = fabs(diff - target);
        if (orb < best_orb) {
            best_orb = orb;
            best_idx = i;
        }
    }

    if (best_idx == (size_t)-1) {
        out->type = OE_ASPECT_NONE;
        return OE_OK;
    }

    max_orb = (max_orb_override > 0.0) ? max_orb_override : kAspectDefs[best_idx].default_minor_orb;

    if (best_orb <= max_orb) {
        out->type = kAspectDefs[best_idx].type;
        out->angle_deg = kAspectDefs[best_idx].angle;
        out->orb_deg = best_orb;
        out->max_orb_deg = max_orb;

        /* Applying / Separating calculation:
           Direct angular distance from lon1 to lon2: delta = lon2 - lon1 (mod [-180, 180])
           Rate of change of delta: d(delta)/dt = speed2 - speed1.
           If delta > 0, diff = delta; diff_rate = speed2 - speed1.
           If delta < 0, diff = -delta; diff_rate = -(speed2 - speed1) = speed1 - speed2.
        */
        if (isfinite(speed1) && isfinite(speed2)) {
            double delta = fmod(lon2 - lon1 + 540.0, 360.0) - 180.0;
            rel_speed = speed2 - speed1;
            diff_rate = (delta >= 0.0) ? rel_speed : -rel_speed;

            /* If current diff < target angle, applying when diff is increasing (diff_rate > 0)
               If current diff > target angle, applying when diff is decreasing (diff_rate < 0) */
            if (diff < out->angle_deg) {
                if (diff_rate > 0.0) out->flags |= OE_ASPECT_APPLYING;
                else out->flags |= OE_ASPECT_SEPARATING;
            } else {
                if (diff_rate < 0.0) out->flags |= OE_ASPECT_APPLYING;
                else out->flags |= OE_ASPECT_SEPARATING;
            }
        }

        if (best_orb < (1.0 / 60.0)) {
            out->flags |= OE_ASPECT_EXACT;
        }
    } else {
        out->type = OE_ASPECT_NONE;
    }

    return OE_OK;
}

oe_status oe_aspect_between_bodies_at_jd(const oe_ephemeris *ephemeris,
                                         oe_body body1, oe_body body2,
                                         double jd_ut,
                                         double max_orb_override,
                                         oe_aspect_info *out) {
    oe_position_result p1, p2;
    oe_status status;
    if (!ephemeris || !out) return OE_ERR_INVALID_ARGUMENT;
    status = oe_position_at_jd(ephemeris, body1, jd_ut, &p1);
    if (status != OE_OK) return status;
    status = oe_position_at_jd(ephemeris, body2, jd_ut, &p2);
    if (status != OE_OK) return status;
    status = oe_aspect_calculate(p1.longitude_deg,
                                 p1.longitude_speed_deg_per_day,
                                 p2.longitude_deg,
                                 p2.longitude_speed_deg_per_day,
                                 max_orb_override, out);
    if (status == OE_OK) {
        out->body1 = (int)body1;
        out->body2 = (int)body2;
    }
    return status;
}

oe_status oe_declination_aspect_calculate(double dec1, double dec_speed1,
                                          double dec2, double dec_speed2,
                                          double max_orb_override,
                                          oe_aspect_info *out) {
    double orb_par, orb_contra, max_orb;
    if (!out || !isfinite(dec1) || !isfinite(dec2)) return OE_ERR_INVALID_ARGUMENT;

    memset(out, 0, sizeof(*out));
    max_orb = (max_orb_override > 0.0) ? max_orb_override : 1.2;

    orb_par = fabs(dec1 - dec2);
    orb_contra = fabs(dec1 + dec2);

    if (orb_par <= max_orb && orb_par <= orb_contra) {
        out->type = OE_ASPECT_PARALLEL;
        out->angle_deg = 0.0;
        out->actual_diff_deg = orb_par;
        out->orb_deg = orb_par;
        out->max_orb_deg = max_orb;
        if (isfinite(dec_speed1) && isfinite(dec_speed2)) {
            double rate = fabs(dec1 + dec_speed1 - (dec2 + dec_speed2)) - orb_par;
            if (rate < 0.0) out->flags |= OE_ASPECT_APPLYING;
            else out->flags |= OE_ASPECT_SEPARATING;
        }
    } else if (orb_contra <= max_orb) {
        out->type = OE_ASPECT_CONTRAPARALLEL;
        out->angle_deg = 0.0;
        out->actual_diff_deg = orb_contra;
        out->orb_deg = orb_contra;
        out->max_orb_deg = max_orb;
        if (isfinite(dec_speed1) && isfinite(dec_speed2)) {
            double rate = fabs(dec1 + dec_speed1 + dec2 + dec_speed2) - orb_contra;
            if (rate < 0.0) out->flags |= OE_ASPECT_APPLYING;
            else out->flags |= OE_ASPECT_SEPARATING;
        }
    } else {
        out->type = OE_ASPECT_NONE;
    }

    return OE_OK;
}

double oe_part_of_fortune(double ascendant_deg, double sun_lon_deg,
                          double moon_lon_deg, int is_night_chart) {
    double pof;
    if (is_night_chart) {
        pof = ascendant_deg + sun_lon_deg - moon_lon_deg;
    } else {
        pof = ascendant_deg + moon_lon_deg - sun_lon_deg;
    }
    return oe_norm_deg(pof);
}
