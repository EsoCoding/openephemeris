#include "oe_internal.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *oe_version(void) { return "0.1.0"; }

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

void oe_ephemeris_close(oe_ephemeris *e) {
    if (!e) return;
    oe_spk_close(&e->planetary);
    if (e->has_chiron) oe_spk_close(&e->chiron);
    free(e);
}

oe_status oe_position(const oe_ephemeris *e, oe_body body,
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
        return oe_apparent_position(e, 2002060, time, out);
    }
    return OE_ERR_INVALID_ARGUMENT;
}
