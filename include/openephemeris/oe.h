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
#define OE_VERSION_MINOR 1
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
OE_API oe_status oe_placidus_houses(const oe_time *time, double latitude_deg,
                                    double longitude_deg, oe_house_result *out);
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

#ifdef __cplusplus
}
#endif
#endif
