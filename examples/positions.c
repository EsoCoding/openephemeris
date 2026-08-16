#include "openephemeris/oe.h"
#include <stdio.h>

int main(void) {
    oe_ephemeris *ephemeris = NULL;
    oe_chart_result chart;
    oe_status status;
    size_t body;

    status = oe_ephemeris_open_default(&ephemeris);
    if (status != OE_OK) {
        fprintf(stderr, "OpenEphemeris data: %s\nRun: cmake --build build --target oe-data\n",
                oe_status_string(status));
        return 1;
    }

    status = oe_chart_from_utc(ephemeris,
                               1984, 6, 23, 5, 51, 0.0,
                               52.3594, 6.4665,
                               &chart);
    if (status != OE_OK) {
        fprintf(stderr, "Chart: %s\n", oe_status_string(status));
        oe_ephemeris_close(ephemeris);
        return 1;
    }

    printf("Ascendant %10.6f   MC %10.6f\n\n",
           chart.houses.ascendant_deg, chart.houses.midheaven_deg);
    for (body = 0; body < OE_BODY_COUNT; ++body) {
        if (chart.position_status[body] == OE_OK) {
            printf("%-16s %10.6f   house %5.2f\n",
                   oe_body_name((oe_body)body),
                   chart.positions[body].longitude_deg,
                   chart.house_positions[body]);
        } else {
            printf("%-16s unavailable (%s)\n",
                   oe_body_name((oe_body)body),
                   oe_status_string((oe_status)chart.position_status[body]));
        }
    }

    oe_ephemeris_close(ephemeris);
    return 0;
}
