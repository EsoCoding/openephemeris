#ifndef OE_INTERNAL_H
#define OE_INTERNAL_H

#include "openephemeris/oe.h"
#include <stddef.h>

#define OE_PI 3.141592653589793238462643383279502884
#define OE_D2R (OE_PI / 180.0)
#define OE_R2D (180.0 / OE_PI)
#define OE_AU_KM 149597870.700
#define OE_C_KM_S 299792.458
#define OE_J2000 2451545.0
#define OE_DAY_S 86400.0
#define OE_MAX_SEGMENTS 64
#define OE_SPK21_MAX_TERMS 25

typedef struct oe_vec3 { double x, y, z; } oe_vec3;
typedef struct oe_state { oe_vec3 p, v; } oe_state;

typedef struct oe_spk_segment {
    double first_et, last_et;
    int target, center, frame, type;
    int64_t first_addr, last_addr;
} oe_spk_segment;

typedef struct oe_spk {
    unsigned char *data;
    size_t size;
    int little_endian;
    oe_spk_segment segments[OE_MAX_SEGMENTS];
    size_t segment_count;
} oe_spk;

struct oe_ephemeris { oe_spk planetary; oe_spk chiron; int has_chiron; };

double oe_norm_deg(double angle);
double oe_mean_obliquity_rad(double jd_tt);
double oe_gmst_deg(double jd_ut1, double jd_tt);
double oe_delta_t_seconds(double year);
oe_status oe_time_from_ut_jd(double jd_ut1, oe_time *out);
oe_status oe_spk_open(oe_spk *spk, const char *path);
void oe_spk_close(oe_spk *spk);
oe_status oe_spk_state(const oe_spk *spk, int target, int center,
                       double et, oe_state *out);
oe_status oe_spk_direct_state(const oe_spk *spk, int target, int center,
                              double et, oe_state *out);
oe_status oe_apparent_position(const oe_ephemeris *e, int target,
                               const oe_time *time, oe_position_result *out);
oe_status oe_lunar_point(const oe_ephemeris *e, oe_body body,
                         const oe_time *time, oe_position_result *out);
oe_status oe_position_time(const oe_ephemeris *e, oe_body body,
                           const oe_time *time, oe_position_result *out);
oe_status oe_houses_time(const oe_time *time, double latitude_deg,
                         double longitude_deg, int house_system,
                         oe_house_result *out);
oe_status oe_house_position_time(const oe_time *time, double latitude_deg,
                                 double longitude_deg, int house_system,
                                 double right_ascension_deg,
                                 double declination_deg, double *out);

#endif
