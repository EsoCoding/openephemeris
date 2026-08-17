#include "oe_internal.h"
#include "erfa.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

#define VERY_SMALL 1e-12

static double sind(double x) { return sin(x * OE_D2R); }
static double cosd(double x) { return cos(x * OE_D2R); }
static double tand(double x) { return tan(x * OE_D2R); }
static double asind(double x) {
    if (x >= 1.0) return 90.0;
    if (x <= -1.0) return -90.0;
    return asin(x) * OE_R2D;
}
static double atand(double x) { return atan(x) * OE_R2D; }

static double degnorm(double x) {
    return oe_norm_deg(x);
}

static double difdeg2n(double p1, double p2) {
    double dif = degnorm(p1 - p2);
    if (dif >= 180.0) dif -= 360.0;
    return dif;
}

static double cotrans(double lon, double lat, double eps) {
    double x, y, z;
    double sinl = sind(lon);
    double cosl = cosd(lon);
    double sinb = sind(lat);
    double cosb = cosd(lat);
    double sine = sind(eps);
    double cose = cosd(eps);
    x = cosb * cosl;
    y = cosb * sinl * cose - sinb * sine;
    z = cosb * sinl * sine + sinb * cose;
    (void)z;
    return degnorm(atan2(y, x) * OE_R2D);
}

static double Asc2(double x, double f, double sine, double cose) {
    double ass, sinx;
    ass = -tand(f) * sine + cose * cosd(x);
    if (fabs(ass) < VERY_SMALL) ass = 0.0;
    sinx = sind(x);
    if (fabs(sinx) < VERY_SMALL) sinx = 0.0;
    if (sinx == 0.0) {
        ass = (ass < 0.0) ? -VERY_SMALL : VERY_SMALL;
    } else if (ass == 0.0) {
        ass = (sinx < 0.0) ? -90.0 : 90.0;
    } else {
        ass = atand(sinx / ass);
    }
    if (ass < 0.0) ass = 180.0 + ass;
    return ass;
}

static double Asc1(double x1, double f, double sine, double cose) {
    int n;
    double ass;
    x1 = degnorm(x1);
    n = (int)(x1 / 90.0) + 1;
    if (fabs(90.0 - f) < VERY_SMALL) return 180.0;
    if (fabs(90.0 + f) < VERY_SMALL) return 0.0;
    if (n == 1) {
        ass = Asc2(x1, f, sine, cose);
    } else if (n == 2) {
        ass = 180.0 - Asc2(180.0 - x1, -f, sine, cose);
    } else if (n == 3) {
        ass = 180.0 + Asc2(x1 - 180.0, -f, sine, cose);
    } else {
        ass = 360.0 - Asc2(360.0 - x1, f, sine, cose);
    }
    ass = degnorm(ass);
    if (fabs(ass - 90.0) < VERY_SMALL) ass = 90.0;
    if (fabs(ass - 180.0) < VERY_SMALL) ass = 180.0;
    if (fabs(ass - 270.0) < VERY_SMALL) ass = 270.0;
    if (fabs(ass - 360.0) < VERY_SMALL) ass = 0.0;
    return ass;
}

static void compute_time_params(const oe_time *t, double lond,
                                double *out_armc, double *out_eps_deg,
                                double *out_sine, double *out_cose) {
    double gst_rad, armc, dpsi, deps, eps_rad, eps_deg;
    gst_rad = eraGst06a(2400000.5, t->jd_ut1 - 2400000.5,
                        2400000.5, t->jd_tt - 2400000.5);
    armc = degnorm(gst_rad * OE_R2D + lond);
    eraNut06a(2400000.5, t->jd_tt - 2400000.5, &dpsi, &deps);
    eps_rad = eraObl06(2400000.5, t->jd_tt - 2400000.5) + deps;
    eps_deg = eps_rad * OE_R2D;
    *out_armc = armc;
    *out_eps_deg = eps_deg;
    *out_sine = sind(eps_deg);
    *out_cose = cosd(eps_deg);
}

oe_status oe_houses(const oe_time *t, double latd, double lond, int house_system, oe_house_result *o) {
    double armc, ekl, sine, cose, tane, tanfi, cosfi;
    double mc, asc, ic, desc, acmc;
    double a, c, f, fh1, fh2, xh1, xh2, rectasc, ad3, sina, cosa;
    int i, niter_max = 100;
    char hsy;

    if (!t || !o || t->struct_size < sizeof(*t) || t->abi_version != OE_ABI_VERSION ||
        !isfinite(latd) || !isfinite(lond) || fabs(latd) >= 90.0 || fabs(lond) > 180.0) {
        return OE_ERR_INVALID_ARGUMENT;
    }

    hsy = (char)toupper((unsigned char)house_system);
    compute_time_params(t, lond, &armc, &ekl, &sine, &cose);

    tane = tand(ekl);
    tanfi = tand(latd);
    cosfi = cosd(latd);

    /* Midheaven (MC) */
    if (fabs(armc - 90.0) > VERY_SMALL && fabs(armc - 270.0) > VERY_SMALL) {
        mc = atand(tand(armc) / cose);
        if (armc > 90.0 && armc <= 270.0) mc = degnorm(mc + 180.0);
    } else {
        mc = (fabs(armc - 90.0) <= VERY_SMALL) ? 90.0 : 270.0;
    }
    mc = degnorm(mc);

    /* Ascendant */
    asc = Asc1(armc + 90.0, latd, sine, cose);
    ic = degnorm(mc + 180.0);
    desc = degnorm(asc + 180.0);

    memset(o, 0, sizeof(*o));
    o->struct_size = sizeof(*o);
    o->abi_version = OE_ABI_VERSION;
    o->ascendant_deg = asc;
    o->midheaven_deg = mc;
    o->armc_deg = armc;
    o->cusps_deg[0] = asc;   /* House 1 */
    o->cusps_deg[9] = mc;    /* House 10 */
    o->cusps_deg[3] = ic;    /* House 4 */
    o->cusps_deg[6] = desc;  /* House 7 */

    switch (hsy) {
    case 'A': /* Equal (Ascendant) */
    case 'E':
        acmc = difdeg2n(asc, mc);
        if (acmc < 0) {
            asc = degnorm(asc + 180.0);
            o->cusps_deg[0] = asc;
        }
        for (i = 2; i <= 12; i++) {
            o->cusps_deg[i - 1] = degnorm(o->cusps_deg[0] + (i - 1) * 30.0);
        }
        break;

    case 'D': /* Equal (MC) */
        o->cusps_deg[9] = mc;
        for (i = 11; i <= 12; i++) o->cusps_deg[i - 1] = degnorm(mc + (i - 10) * 30.0);
        for (i = 1; i <= 9; i++) o->cusps_deg[i - 1] = degnorm(mc + (i + 2) * 30.0);
        break;

    case 'W': /* Whole Sign */
        acmc = difdeg2n(asc, mc);
        if (acmc < 0) asc = degnorm(asc + 180.0);
        o->cusps_deg[0] = asc - fmod(asc, 30.0);
        for (i = 2; i <= 12; i++) {
            o->cusps_deg[i - 1] = degnorm(o->cusps_deg[0] + (i - 1) * 30.0);
        }
        break;

    case 'V': /* Equal Vehlow */
        o->cusps_deg[0] = degnorm(asc - 15.0);
        for (i = 2; i <= 12; i++) {
            o->cusps_deg[i - 1] = degnorm(o->cusps_deg[0] + (i - 1) * 30.0);
        }
        break;

    case 'O': /* Porphyry */
    porphyry_label:
        acmc = difdeg2n(asc, mc);
        if (acmc < 0) {
            asc = degnorm(asc + 180.0);
            o->cusps_deg[0] = asc;
            acmc = difdeg2n(asc, mc);
        }
        o->cusps_deg[0] = asc;
        o->cusps_deg[9] = mc;
        o->cusps_deg[1] = degnorm(asc + (180.0 - acmc) / 3.0);
        o->cusps_deg[2] = degnorm(asc + (180.0 - acmc) / 3.0 * 2.0);
        o->cusps_deg[10] = degnorm(mc + acmc / 3.0);
        o->cusps_deg[11] = degnorm(mc + acmc / 3.0 * 2.0);
        o->cusps_deg[3] = degnorm(mc + 180.0);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[6] = degnorm(asc + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        break;

    case 'K': /* Koch */
        if (fabs(latd) >= 90.0 - ekl) {
            goto porphyry_label;
        }
        sina = sind(mc) * sine / cosfi;
        if (sina > 1.0) sina = 1.0;
        if (sina < -1.0) sina = -1.0;
        cosa = sqrt(1.0 - sina * sina);
        c = atand(tanfi / cosa);
        ad3 = asind(sind(c) * sina) / 3.0;
        o->cusps_deg[10] = Asc1(armc + 30.0 - 2.0 * ad3, latd, sine, cose);
        o->cusps_deg[11] = Asc1(armc + 60.0 - ad3, latd, sine, cose);
        o->cusps_deg[1] = Asc1(armc + 120.0 + ad3, latd, sine, cose);
        o->cusps_deg[2] = Asc1(armc + 150.0 + 2.0 * ad3, latd, sine, cose);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        break;

    case 'R': /* Regiomontanus */
        fh1 = atand(tanfi * 0.5);
        fh2 = atand(tanfi * cosd(30.0));
        o->cusps_deg[10] = Asc1(30.0 + armc, fh1, sine, cose);
        o->cusps_deg[11] = Asc1(60.0 + armc, fh2, sine, cose);
        o->cusps_deg[1] = Asc1(120.0 + armc, fh2, sine, cose);
        o->cusps_deg[2] = Asc1(150.0 + armc, fh1, sine, cose);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        if (fabs(latd) >= 90.0 - ekl) {
            acmc = difdeg2n(asc, mc);
            if (acmc < 0) {
                for (i = 0; i < 12; i++) o->cusps_deg[i] = degnorm(o->cusps_deg[i] + 180.0);
            }
        }
        break;

    case 'C': /* Campanus */
        fh1 = asind(sind(latd) / 2.0);
        fh2 = asind(sqrt(3.0) / 2.0 * sind(latd));
        if (fabs(cosfi) == 0.0) {
            xh1 = xh2 = (latd > 0) ? 90.0 : 270.0;
        } else {
            xh1 = atand(sqrt(3.0) / cosfi);
            xh2 = atand(1.0 / sqrt(3.0) / cosfi);
        }
        o->cusps_deg[10] = Asc1(armc + 90.0 - xh1, fh1, sine, cose);
        o->cusps_deg[11] = Asc1(armc + 90.0 - xh2, fh2, sine, cose);
        o->cusps_deg[1] = Asc1(armc + 90.0 + xh2, fh2, sine, cose);
        o->cusps_deg[2] = Asc1(armc + 90.0 + xh1, fh1, sine, cose);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        if (fabs(latd) >= 90.0 - ekl) {
            acmc = difdeg2n(asc, mc);
            if (acmc < 0) {
                for (i = 0; i < 12; i++) o->cusps_deg[i] = degnorm(o->cusps_deg[i] + 180.0);
            }
        }
        break;

    case 'T': /* Topocentric */
        fh1 = atand(tanfi / 3.0);
        fh2 = atand(tanfi * 2.0 / 3.0);
        o->cusps_deg[10] = Asc1(30.0 + armc, fh1, sine, cose);
        o->cusps_deg[11] = Asc1(60.0 + armc, fh2, sine, cose);
        o->cusps_deg[1] = Asc1(120.0 + armc, fh2, sine, cose);
        o->cusps_deg[2] = Asc1(150.0 + armc, fh1, sine, cose);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        if (fabs(latd) >= 90.0 - ekl) {
            acmc = difdeg2n(asc, mc);
            if (acmc < 0) {
                for (i = 0; i < 12; i++) o->cusps_deg[i] = degnorm(o->cusps_deg[i] + 180.0);
            }
        }
        break;

    case 'B': { /* Alcabitius */
        double dek, r, sna, sda, sn3, sd3;
        dek = asind(sind(asc) * sine);
        r = -tanfi * tand(dek);
        if (r < -1.0) r = -1.0;
        if (r > 1.0) r = 1.0;
        sda = acos(r) * OE_R2D;
        sna = 180.0 - sda;
        sd3 = sda / 3.0;
        sn3 = sna / 3.0;
        o->cusps_deg[10] = Asc1(degnorm(armc + sd3), 0.0, sine, cose);
        o->cusps_deg[11] = Asc1(degnorm(armc + 2.0 * sd3), 0.0, sine, cose);
        o->cusps_deg[1] = Asc1(degnorm(armc + 180.0 - 2.0 * sn3), 0.0, sine, cose);
        o->cusps_deg[2] = Asc1(degnorm(armc + 180.0 - sn3), 0.0, sine, cose);
        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        break;
    }

    case 'M': { /* Morinus */
        for (i = 1; i <= 12; i++) {
            int j = (i + 9) % 12; /* house 10 is index 9, house 11 is 10, etc. */
            double ra_pt = degnorm(armc + i * 30.0);
            o->cusps_deg[j] = cotrans(ra_pt, 0.0, -ekl);
        }
        break;
    }

    case 'X': { /* Meridian / Axial */
        for (i = 1; i <= 12; i++) {
            int j = (i + 9) % 12;
            double ra_pt = degnorm(armc + i * 30.0);
            o->cusps_deg[j] = Asc1(ra_pt, 0.0, sine, cose);
        }
        break;
    }

    case 'P': /* Placidus */
    default: {
        double cuspsv, tant;
        if (fabs(latd) >= 90.0 - ekl) {
            goto porphyry_label;
        }
        a = asind(tanfi * tane);
        fh1 = atand(sind(a / 3.0) / tane);
        fh2 = atand(sind(a * 2.0 / 3.0) / tane);

        /* House 11 */
        rectasc = degnorm(30.0 + armc);
        tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
        if (fabs(tant) < VERY_SMALL) {
            o->cusps_deg[10] = rectasc;
        } else {
            f = atand(sind(asind(tanfi * tant) / 3.0) / tant);
            o->cusps_deg[10] = Asc1(rectasc, f, sine, cose);
            cuspsv = 0.0;
            for (i = 1; i <= niter_max; i++) {
                tant = tand(asind(sine * sind(o->cusps_deg[10])));
                if (fabs(tant) < VERY_SMALL) {
                    o->cusps_deg[10] = rectasc;
                    break;
                }
                f = atand(sind(asind(tanfi * tant) / 3.0) / tant);
                o->cusps_deg[10] = Asc1(rectasc, f, sine, cose);
                if (i > 1 && fabs(difdeg2n(o->cusps_deg[10], cuspsv)) < (1.0 / 360000.0)) break;
                cuspsv = o->cusps_deg[10];
            }
        }

        /* House 12 */
        rectasc = degnorm(60.0 + armc);
        tant = tand(asind(sine * sind(Asc1(rectasc, fh2, sine, cose))));
        if (fabs(tant) < VERY_SMALL) {
            o->cusps_deg[11] = rectasc;
        } else {
            f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
            o->cusps_deg[11] = Asc1(rectasc, f, sine, cose);
            cuspsv = 0.0;
            for (i = 1; i <= niter_max; i++) {
                tant = tand(asind(sine * sind(o->cusps_deg[11])));
                if (fabs(tant) < VERY_SMALL) {
                    o->cusps_deg[11] = rectasc;
                    break;
                }
                f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
                o->cusps_deg[11] = Asc1(rectasc, f, sine, cose);
                if (i > 1 && fabs(difdeg2n(o->cusps_deg[11], cuspsv)) < (1.0 / 360000.0)) break;
                cuspsv = o->cusps_deg[11];
            }
        }

        /* House 2 */
        rectasc = degnorm(120.0 + armc);
        tant = tand(asind(sine * sind(Asc1(rectasc, fh2, sine, cose))));
        if (fabs(tant) < VERY_SMALL) {
            o->cusps_deg[1] = rectasc;
        } else {
            f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
            o->cusps_deg[1] = Asc1(rectasc, f, sine, cose);
            cuspsv = 0.0;
            for (i = 1; i <= niter_max; i++) {
                tant = tand(asind(sine * sind(o->cusps_deg[1])));
                if (fabs(tant) < VERY_SMALL) {
                    o->cusps_deg[1] = rectasc;
                    break;
                }
                f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
                o->cusps_deg[1] = Asc1(rectasc, f, sine, cose);
                if (i > 1 && fabs(difdeg2n(o->cusps_deg[1], cuspsv)) < (1.0 / 360000.0)) break;
                cuspsv = o->cusps_deg[1];
            }
        }

        /* House 3 */
        rectasc = degnorm(150.0 + armc);
        tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
        if (fabs(tant) < VERY_SMALL) {
            o->cusps_deg[2] = rectasc;
        } else {
            f = atand(sind(asind(tanfi * tant) / 3.0) / tant);
            o->cusps_deg[2] = Asc1(rectasc, f, sine, cose);
            cuspsv = 0.0;
            for (i = 1; i <= niter_max; i++) {
                tant = tand(asind(sine * sind(o->cusps_deg[2])));
                if (fabs(tant) < VERY_SMALL) {
                    o->cusps_deg[2] = rectasc;
                    break;
                }
                f = atand(sind(asind(tanfi * tant) / 3.0) / tant);
                o->cusps_deg[2] = Asc1(rectasc, f, sine, cose);
                if (i > 1 && fabs(difdeg2n(o->cusps_deg[2], cuspsv)) < (1.0 / 360000.0)) break;
                cuspsv = o->cusps_deg[2];
            }
        }

        o->cusps_deg[4] = degnorm(o->cusps_deg[10] + 180.0);
        o->cusps_deg[5] = degnorm(o->cusps_deg[11] + 180.0);
        o->cusps_deg[7] = degnorm(o->cusps_deg[1] + 180.0);
        o->cusps_deg[8] = degnorm(o->cusps_deg[2] + 180.0);
        break;
    }
    }

    for (i = 0; i < 12; i++) {
        o->cusps_deg[i] = degnorm(o->cusps_deg[i]);
    }
    return OE_OK;
}

oe_status oe_placidus_houses(const oe_time *t, double latd, double lond, oe_house_result *o) {
    return oe_houses(t, latd, lond, OE_HOUSE_PLACIDUS, o);
}

oe_status oe_house_position(const oe_time *t,
                            double latd,
                            double lond,
                            int house_system,
                            double rad,
                            double decd,
                            double *out) {
    double armc, ekl, sine, cose, tanfi, phi, dec, h, s, n, x;
    char hsy;

    if (!t || !out || !isfinite(latd) || !isfinite(lond) ||
        !isfinite(rad) || !isfinite(decd) || fabs(latd) >= 90.0 ||
        fabs(decd) > 90.0 || fabs(lond) > 180.0) {
        return OE_ERR_INVALID_ARGUMENT;
    }

    hsy = (char)toupper((unsigned char)house_system);
    compute_time_params(t, lond, &armc, &ekl, &sine, &cose);

    if (hsy == 'P' || hsy == 'T' || hsy == 'O' || hsy == 'A' || hsy == 'E' || hsy == 'W' || hsy == 'D') {
        /* Standard semi-arc / placidian quadrant computation */
        phi = latd * OE_D2R;
        dec = decd * OE_D2R;
        tanfi = tan(phi) * tan(dec);
        if (fabs(tanfi) >= 1.0) {
            return OE_ERR_HOUSES_UNDEFINED;
        }
        s = acos(-tanfi);
        n = OE_PI - s;
        h = difdeg2n(armc, rad) * OE_D2R;

        if (h >= 0 && h <= s) {
            x = 10.0 - 3.0 * h / s;
        } else if (h > s) {
            x = 7.0 - 3.0 * (h - s) / n;
        } else if (h >= -s) {
            x = 10.0 + 3.0 * (-h) / s;
        } else {
            x = 4.0 - 3.0 * (h + OE_PI) / n;
        }

        while (x >= 13.0) x -= 12.0;
        while (x < 1.0) x += 12.0;
        *out = x;
        return OE_OK;
    }

    /* Fallback to cusp-based placement for other systems */
    {
        oe_house_result hr;
        oe_status status = oe_houses(t, latd, lond, house_system, &hr);
        double ecl_lon, ecl_lat, dist_diff;
        int cur_house;
        if (status != OE_OK) return status;
        /* convert (RA, Dec) to (ecl_lon, ecl_lat) */
        ecl_lon = cotrans(rad, decd, ekl);
        (void)ecl_lat;
        for (cur_house = 0; cur_house < 12; cur_house++) {
            double c1 = hr.cusps_deg[cur_house];
            double c2 = hr.cusps_deg[(cur_house + 1) % 12];
            double span = degnorm(c2 - c1);
            double offset = degnorm(ecl_lon - c1);
            if (offset < span) {
                *out = (double)(cur_house + 1) + offset / span;
                return OE_OK;
            }
        }
        dist_diff = degnorm(ecl_lon - hr.cusps_deg[0]);
        *out = 1.0 + dist_diff / 30.0;
        return OE_OK;
    }
}

oe_status oe_placidus_house_position(const oe_time *t,
                                     double latd,
                                     double lond,
                                     double rad,
                                     double decd,
                                     double *out) {
    return oe_house_position(t, latd, lond, OE_HOUSE_PLACIDUS, rad, decd, out);
}
