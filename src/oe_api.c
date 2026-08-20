#include "oe_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *oe_version(void) { return "0.4.0"; }

const char *oe_body_name(oe_body body) {
    static const char *names[OE_BODY_COUNT] = {
        "Sun", "Moon", "Mercury", "Venus", "Mars", "Jupiter", "Saturn",
        "Uranus", "Neptune", "Pluto", "Mean Node", "True Node",
        "Mean South Node", "True South Node", "Mean Lilith", "True Lilith",
        "Chiron"
    };
    return body >= OE_SUN && body <= OE_CHIRON ? names[body] : "Unknown";
}

const char *oe_status_string(oe_status s) {
    switch (s) {
    case OE_OK: return "success";
    case OE_ERR_INVALID_ARGUMENT: return "invalid argument";
    case OE_ERR_IO: return "I/O error";
    case OE_ERR_BAD_KERNEL: return "invalid SPK kernel";
    case OE_ERR_UNSUPPORTED_KERNEL: return "unsupported SPK segment type";
    case OE_ERR_NO_COVERAGE: return "no ephemeris coverage";
    case OE_ERR_TIME_RANGE: return "time outside supported range";
    case OE_ERR_HOUSES_UNDEFINED: return "Placidus houses undefined";
    case OE_ERR_NUMERIC: return "numeric failure";
    default: return "unknown error";
    }
}

oe_status oe_ephemeris_open(const char *planetary, const char *chiron,
                            oe_ephemeris **out) {
    oe_ephemeris *e;
    oe_status s;
    if (!planetary || !out) return OE_ERR_INVALID_ARGUMENT;
    *out = NULL;
    e = (oe_ephemeris *)calloc(1, sizeof(*e));
    if (!e) return OE_ERR_IO;
    s = oe_spk_open(&e->planetary, planetary);
    if (s != OE_OK) { free(e); return s; }
    if (chiron) {
        s = oe_spk_open(&e->chiron, chiron);
        if (s != OE_OK) { oe_spk_close(&e->planetary); free(e); return s; }
        e->has_chiron = 1;
    }
    *out = e;
    return OE_OK;
}

static oe_status open_from_directory(const char *directory, oe_ephemeris **out) {
    char planetary[4096], chiron[4096];
    FILE *file;
    int n;
    if (!directory || !*directory) return OE_ERR_INVALID_ARGUMENT;
    n = snprintf(planetary, sizeof(planetary), "%s/%s", directory, "de440.bsp");
    if (n < 0 || (size_t)n >= sizeof(planetary)) return OE_ERR_INVALID_ARGUMENT;
    n = snprintf(chiron, sizeof(chiron), "%s/%s", directory,
                 "chiron-2060-1800-2200.bsp");
    if (n < 0 || (size_t)n >= sizeof(chiron)) return OE_ERR_INVALID_ARGUMENT;
    file = fopen(chiron, "rb");
    if (file) { fclose(file); return oe_ephemeris_open(planetary, chiron, out); }
    return oe_ephemeris_open(planetary, NULL, out);
}

oe_status oe_ephemeris_open_default(oe_ephemeris **out) {
    static const char *directories[] = { "data", ".", "../data" };
    const char *environment;
    oe_status status;
    size_t i;
    if (!out) return OE_ERR_INVALID_ARGUMENT;
    *out = NULL;
    environment = getenv("OE_DATA_PATH");
    if (environment && *environment) {
        status = open_from_directory(environment, out);
        if (status == OE_OK || status != OE_ERR_IO) return status;
    }
    for (i = 0; i < sizeof(directories) / sizeof(directories[0]); ++i) {
        status = open_from_directory(directories[i], out);
        if (status == OE_OK) return OE_OK;
        if (status != OE_ERR_IO) return status;
    }
    return OE_ERR_IO;
}

void oe_ephemeris_close(oe_ephemeris *e) {
    if (!e) return;
    oe_spk_close(&e->planetary);
    if (e->has_chiron) oe_spk_close(&e->chiron);
    free(e);
}

oe_status oe_position_time(const oe_ephemeris *e, oe_body body,
                           const oe_time *time, oe_position_result *out) {
    static const int ids[] = {10, 301, 1, 2, 4, 5, 6, 7, 8, 9};
    if (!e || !time || !out || time->struct_size < sizeof(*time) ||
        time->abi_version != OE_ABI_VERSION || !isfinite(time->jd_tt))
        return OE_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out));
    out->struct_size = sizeof(*out); out->abi_version = OE_ABI_VERSION;
    if (body >= OE_SUN && body <= OE_PLUTO)
        return oe_apparent_position(e, ids[body], time, out);
    if (body >= OE_MEAN_NODE && body <= OE_TRUE_LILITH)
        return oe_lunar_point(e, body, time, out);
    if (body == OE_CHIRON) {
        if (!e->has_chiron) return OE_ERR_NO_COVERAGE;
        return oe_apparent_position(e, 20002060, time, out);
    }
    return OE_ERR_INVALID_ARGUMENT;
}

oe_status oe_position_at_jd(const oe_ephemeris *e, oe_body body,
                            double jd_ut,
                            oe_position_result *out) {
    oe_time time;
    oe_status status = oe_time_from_ut_jd(jd_ut, &time);
    if (status != OE_OK) return status;
    return oe_position_time(e, body, &time, out);
}
