/**
 * OpenEphemeris Example 6: Fixed Stars
 *
 * Demonstrates:
 * - Enumerating the built-in fixed-star catalogue.
 * - Name lookup, including a common Bayer alias.
 * - Apparent ecliptic-of-date longitude and latitude.
 */

#include "openephemeris/oe.h"
#include <math.h>
#include <stdio.h>

int main(void) {
    oe_time time;
    const oe_fixed_star *star;
    oe_position_result position;
    size_t i;

    if (oe_time_from_utc(2000, 1, 1, 12, 0, 0.0, NAN, &time) != OE_OK) {
        fprintf(stderr, "Could not create time\n");
        return 1;
    }

    printf("OpenEphemeris - Fixed Stars\n");
    printf("Date: 2000-01-01 12:00:00 UTC\n\n");
    printf("Catalogue entries: %zu\n", oe_fixed_star_count());
    printf("%-16s %-6s %8s %10s %10s\n", "Name", "Const.", "Mag.",
           "Longitude", "Latitude");

    for (i = 0; i < oe_fixed_star_count(); ++i) {
        star = oe_fixed_star_at(i);
        if (star && oe_fixed_star_position(star, &time, &position) == OE_OK) {
            printf("%-16s %-6s %8.2f %9.4f deg %9.4f deg\n",
                   star->name, star->constellation, star->magnitude,
                   position.longitude_deg, position.latitude_deg);
        }
    }

    star = oe_fixed_star_find("Alpha Tauri");
    if (star) {
        printf("\nAlias lookup: Alpha Tauri -> %s\n", star->name);
    }
    star = oe_fixed_star_find("Aldebaran");
    if (!star || oe_fixed_star_position(star, &time, &position) != OE_OK) {
        fprintf(stderr, "Aldebaran lookup failed\n");
        return 1;
    }
    printf("Aldebaran: %.4f deg longitude, %.4f deg latitude\n",
           position.longitude_deg, position.latitude_deg);
    return 0;
}
