#ifndef OPENEPHEMERIS_OE_H
#define OPENEPHEMERIS_OE_H

#include <stddef.h>
#include <stdint.h>

#if defined(OE_STATIC)
# define OE_API
#elif defined(_WIN32) && defined(OE_BUILDING_DLL)
# define OE_API __declspec(dllexport)
#elif defined(_WIN32)
# define OE_API __declspec(dllimport)
#elif defined(__GNUC__)
# define OE_API __attribute__((visibility("default")))
#else
# define OE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OE_ABI_VERSION 1u
#define OE_VERSION_MAJOR 0
#define OE_VERSION_MINOR 2
#define OE_VERSION_PATCH 0

typedef struct oe_ephemeris oe_ephemeris;

typedef enum oe_status {
    OE_OK = 0,
    OE_ERR_INVALID_ARGUMENT = -1,
    OE_ERR_IO = -2,
    OE_ERR_BAD_KERNEL = -3,
    OE_ERR_UNSUPPORTED_KERNEL = -4,
    OE_ERR_NO_COVERAGE = -5,
    OE_ERR_TIME_RANGE = -6,
    OE_ERR_HOUSES_UNDEFINED = -7,
    OE_ERR_NUMERIC = -8
} oe_status;

typedef enum oe_time_quality {
    OE_TIME_EXACT = 0,
    OE_TIME_LEAP_SECONDS_KNOWN = 1u << 0,
    OE_TIME_DUT1_MODELED = 1u << 1,
    OE_TIME_DELTA_T_MODELED = 1u << 2,
    OE_TIME_FUTURE_UTC = 1u << 3
} oe_time_quality;

typedef enum oe_body {
    OE_SUN = 0, OE_MOON, OE_MERCURY, OE_VENUS, OE_MARS, OE_JUPITER,
    OE_SATURN, OE_URANUS, OE_NEPTUNE, OE_PLUTO, OE_MEAN_NODE,
    OE_TRUE_NODE, OE_MEAN_SOUTH_NODE, OE_TRUE_SOUTH_NODE,
    OE_MEAN_LILITH, OE_TRUE_LILITH, OE_CHIRON
} oe_body;

#define OE_BODY_COUNT 17u

typedef struct oe_time {
    uint32_t struct_size;
    uint32_t abi_version;
    double jd_tt;
    double jd_ut1;
    uint32_t quality;
    uint32_t reserved;
} oe_time;

typedef struct oe_position_result {
    uint32_t struct_size;
    uint32_t abi_version;
    double longitude_deg;
    double latitude_deg;
    double distance_au;
    double longitude_speed_deg_per_day;
    double latitude_speed_deg_per_day;
    double distance_speed_au_per_day;
    uint32_t flags;
    uint32_t reserved;
} oe_position_result;

typedef struct oe_house_result {
    uint32_t struct_size;
    uint32_t abi_version;
    double cusps_deg[12];
    double ascendant_deg;
    double midheaven_deg;
    double armc_deg;
    uint32_t flags;
    uint32_t reserved;
} oe_house_result;

typedef struct oe_chart_result {
    uint32_t struct_size;
    uint32_t abi_version;
    oe_time time;
    oe_house_result houses;
    oe_position_result positions[OE_BODY_COUNT];
    double house_positions[OE_BODY_COUNT];
    int32_t position_status[OE_BODY_COUNT];
    int32_t house_status[OE_BODY_COUNT];
    uint32_t flags;
    uint32_t reserved;
} oe_chart_result;

typedef enum oe_house_system {
    OE_HOUSE_PLACIDUS = 'P',
    OE_HOUSE_KOCH = 'K',
    OE_HOUSE_PORPHYRY = 'O',
    OE_HOUSE_REGIOMONTANUS = 'R',
    OE_HOUSE_CAMPANUS = 'C',
    OE_HOUSE_EQUAL = 'A',
    OE_HOUSE_WHOLE_SIGN = 'W',
    OE_HOUSE_ALCABITIUS = 'B',
    OE_HOUSE_TOPOCENTRIC = 'T',
    OE_HOUSE_MORINUS = 'M',
    OE_HOUSE_MERIDIAN = 'X',
    OE_HOUSE_VEHLOW = 'V',
    OE_HOUSE_EQUAL_MC = 'D'
} oe_house_system;

typedef enum oe_zodiac_sign {
    OE_SIGN_ARIES = 0,
    OE_SIGN_TAURUS,
    OE_SIGN_GEMINI,
    OE_SIGN_CANCER,
    OE_SIGN_LEO,
    OE_SIGN_VIRGO,
    OE_SIGN_LIBRA,
    OE_SIGN_SCORPIO,
    OE_SIGN_SAGITTARIUS,
    OE_SIGN_CAPRICORN,
    OE_SIGN_AQUARIUS,
    OE_SIGN_PISCES
} oe_zodiac_sign;

typedef enum oe_element {
    OE_ELEMENT_FIRE = 0,
    OE_ELEMENT_EARTH,
    OE_ELEMENT_AIR,
    OE_ELEMENT_WATER
} oe_element;

typedef enum oe_modality {
    OE_MODALITY_CARDINAL = 0,
    OE_MODALITY_FIXED,
    OE_MODALITY_MUTABLE
} oe_modality;

typedef enum oe_aspect_type {
    OE_ASPECT_NONE = 0,
    OE_ASPECT_CONJUNCTION,    /* 0° */
    OE_ASPECT_SEMISEXTILE,    /* 30° */
    OE_ASPECT_SEMISQUARE,     /* 45° */
    OE_ASPECT_SEXTILE,        /* 60° */
    OE_ASPECT_QUINTILE,       /* 72° */
    OE_ASPECT_SQUARE,         /* 90° */
    OE_ASPECT_TRINE,          /* 120° */
    OE_ASPECT_SESQUIQUADRATE, /* 135° */
    OE_ASPECT_BIQUINTILE,     /* 144° */
    OE_ASPECT_QUINCUNX,       /* 150° */
    OE_ASPECT_OPPOSITION,     /* 180° */
    OE_ASPECT_PARALLEL,       /* Equal declination */
    OE_ASPECT_CONTRAPARALLEL  /* Opposite declination */
} oe_aspect_type;

typedef enum oe_aspect_flag {
    OE_ASPECT_APPLYING  = 1u << 0,
    OE_ASPECT_SEPARATING = 1u << 1,
    OE_ASPECT_EXACT     = 1u << 2
} oe_aspect_flag;

typedef enum oe_ayanamsa_mode {
    OE_AYANAMSA_FAGAN_BRADLEY = 0,
    OE_AYANAMSA_LAHIRI,
    OE_AYANAMSA_RAMAN,
    OE_AYANAMSA_KRISHNAMURTI,
    OE_AYANAMSA_YUKTESHWAR,
    OE_AYANAMSA_TRUE_CITRA,
    OE_AYANAMSA_TRUE_REVATI,
    OE_AYANAMSA_TRUE_PUSHYA,
    OE_AYANAMSA_USER
} oe_ayanamsa_mode;

typedef struct oe_fixed_star {
    uint32_t struct_size;
    uint32_t abi_version;
    const char *name;
    const char *constellation;
    double magnitude;
    double ra_j2000_deg;
    double dec_j2000_deg;
    double proper_motion_ra_mas_year;
    double proper_motion_dec_mas_year;
} oe_fixed_star;

typedef struct oe_sidereal_result {
    uint32_t struct_size;
    uint32_t abi_version;
    double ayanamsa_deg;
    double tropical_longitude_deg;
    double sidereal_longitude_deg;
    int nakshatra;
    int pada;
} oe_sidereal_result;

typedef enum oe_search_kind {
    OE_SEARCH_TRANSIT = 0,
    OE_SEARCH_INGRESS,
    OE_SEARCH_SOLAR_RETURN,
    OE_SEARCH_LUNAR_RETURN
} oe_search_kind;

typedef struct oe_search_result {
    uint32_t struct_size;
    uint32_t abi_version;
    oe_time time;
    oe_position_result position;
    double target_longitude_deg;
    double residual_deg;
    uint32_t flags;
} oe_search_result;

typedef enum oe_eclipse_type {
    OE_ECLIPSE_SOLAR = 0,
    OE_ECLIPSE_LUNAR
} oe_eclipse_type;

typedef struct oe_eclipse_result {
    uint32_t struct_size;
    uint32_t abi_version;
    oe_eclipse_type type;
    oe_time maximum;
    double longitude_deg;
    double latitude_deg;
    double magnitude;
    int totality;
} oe_eclipse_result;

typedef struct oe_aspect_info {
    int body1;                  /* oe_body or angle (-1 for ASC, -2 for MC) */
    int body2;
    oe_aspect_type type;
    double angle_deg;           /* Exact nominal aspect angle */
    double actual_diff_deg;     /* Measured separation */
    double orb_deg;             /* Absolute deviation from exact aspect */
    double max_orb_deg;         /* Max allowed orb */
    uint32_t flags;             /* OE_ASPECT_APPLYING / SEPARATING / EXACT */
} oe_aspect_info;

OE_API const char *oe_version(void);
OE_API const char *oe_status_string(oe_status status);
OE_API oe_status oe_ephemeris_open(const char *planetary_kernel,
                                   const char *chiron_kernel,
                                   oe_ephemeris **out);
OE_API oe_status oe_ephemeris_open_default(oe_ephemeris **out);
OE_API void oe_ephemeris_close(oe_ephemeris *ephemeris);
OE_API const char *oe_body_name(oe_body body);
OE_API oe_status oe_time_from_jd(double jd_tt, double jd_ut1, oe_time *out);
OE_API oe_status oe_time_from_utc(int year, int month, int day, int hour,
                                  int minute, double second, double dut1_seconds,
                                  oe_time *out);
OE_API oe_status oe_position(const oe_ephemeris *ephemeris, oe_body body,
                             const oe_time *time, oe_position_result *out);
OE_API oe_status oe_houses(const oe_time *time, double latitude_deg,
                           double longitude_deg, int house_system,
                           oe_house_result *out);
OE_API oe_status oe_placidus_houses(const oe_time *time, double latitude_deg,
                                    double longitude_deg, oe_house_result *out);
OE_API oe_status oe_house_position(const oe_time *time,
                                   double latitude_deg,
                                   double longitude_deg,
                                   int house_system,
                                   double right_ascension_deg,
                                   double declination_deg,
                                   double *house_position);
OE_API oe_status oe_placidus_house_position(const oe_time *time,
                                            double latitude_deg,
                                            double longitude_deg,
                                            double right_ascension_deg,
                                            double declination_deg,
                                            double *house_position);
OE_API oe_status oe_chart_from_utc(const oe_ephemeris *ephemeris,
                                   int year, int month, int day,
                                   int hour, int minute, double second,
                                   double latitude_deg, double longitude_deg,
                                   oe_chart_result *out);

/* Astrological analysis & computations */
OE_API const char *oe_sign_name(oe_zodiac_sign sign);
OE_API const char *oe_sign_symbol(oe_zodiac_sign sign);
OE_API oe_zodiac_sign oe_longitude_sign(double longitude_deg);
OE_API double oe_longitude_in_sign(double longitude_deg);
OE_API oe_element oe_sign_element(oe_zodiac_sign sign);
OE_API oe_modality oe_sign_modality(oe_zodiac_sign sign);

OE_API const char *oe_aspect_name(oe_aspect_type type);
OE_API const char *oe_aspect_symbol(oe_aspect_type type);
OE_API double oe_aspect_angle(oe_aspect_type type);
OE_API double oe_aspect_default_orb(oe_aspect_type type, int body1, int body2);
OE_API oe_status oe_aspect_calculate(double lon1, double speed1,
                                     double lon2, double speed2,
                                     double max_orb_override,
                                     oe_aspect_info *out);
OE_API oe_status oe_declination_aspect_calculate(double dec1, double dec_speed1,
                                                double dec2, double dec_speed2,
                                                double max_orb_override,
                                                oe_aspect_info *out);
OE_API size_t oe_chart_aspects(const oe_chart_result *chart,
                               oe_aspect_info *out_aspects,
                               size_t max_count);
OE_API double oe_part_of_fortune(double ascendant_deg, double sun_lon_deg,
                                 double moon_lon_deg, int is_night_chart);

/* Fixed stars and sidereal astrology. */
OE_API size_t oe_fixed_star_count(void);
OE_API const oe_fixed_star *oe_fixed_star_at(size_t index);
OE_API const oe_fixed_star *oe_fixed_star_find(const char *name);
OE_API oe_status oe_fixed_star_position(const oe_fixed_star *star,
                                        const oe_time *time,
                                        oe_position_result *out);
OE_API double oe_ayanamsa(const oe_time *time, oe_ayanamsa_mode mode,
                          double user_value_deg);
OE_API oe_status oe_sidereal_position(const oe_ephemeris *ephemeris,
                                      oe_body body, const oe_time *time,
                                      oe_ayanamsa_mode mode,
                                      double user_value_deg,
                                      oe_sidereal_result *out);
OE_API oe_status oe_sidereal_houses(const oe_time *time, double latitude_deg,
                                    double longitude_deg, int house_system,
                                    oe_ayanamsa_mode mode, double user_value_deg,
                                    oe_house_result *out);

/* Shared event search engine. direction is +1 or -1 and max_days is a bound. */
OE_API oe_status oe_transit_search(const oe_ephemeris *ephemeris,
                                   oe_body body, double target_longitude_deg,
                                   const oe_time *start, int direction,
                                   double max_days, oe_search_result *out);
OE_API oe_status oe_return_search(const oe_ephemeris *ephemeris,
                                  oe_body body, double natal_longitude_deg,
                                  const oe_time *start, int direction,
                                  double max_days, oe_search_result *out);
OE_API oe_status oe_eclipse_search(const oe_ephemeris *ephemeris,
                                   oe_eclipse_type type, const oe_time *start,
                                   int direction, double max_days,
                                   oe_eclipse_result *out);

#ifdef __cplusplus
}
#endif
#endif
