# OpenEphemeris

OpenEphemeris is a clean-room, permissively licensed C11 calculation library
for high-precision astrology. It reads official JPL SPK kernels
directly and exposes a small, versioned `oe_` C ABI. The core never downloads
data, uses global mutable state, silently changes ephemeris, or substitutes a
different house system.

> Current release status: **0.4.0 development preview**, not 1.0. Planetary
> DE440 calculations, Chiron, houses, fixed stars, sidereal calculations and
> first transit/event APIs are operational. Broad independent validation
> remains a documented 1.0 gate. See
> [docs/STATUS.md](docs/STATUS.md).

## Implemented scope

- apparent geocentric tropical longitude, latitude, distance, and speeds;
- Sun, Moon, Mercury through Pluto using a JPL DE440 kernel;
- Chiron using a pinned JPL Horizons type-21 kernel for 1800--2200;
- mean/true lunar nodes and mean/true Black Moon Lilith;
- all currently implemented house systems, Ascendant, MC, ARMC, and geometric
  house position;
- a built-in bright fixed-star catalogue with proper motion and apparent
  ecliptic-of-date positions;
- ayanamsa modes, sidereal positions/houses, and nakshatra/pada output;
- bounded planetary transit/return search and a preliminary eclipse search;
- explicit UTC/TT/UT1 handling and modeled-time quality flags;
- endian-aware, bounds-checked DAF/SPK type 2, 3, and 21 input;
- static and shared libraries on Linux and Windows-oriented CMake builds.

The code is MIT licensed. The vendored ERFA 2.0.1 dependency is BSD-3-Clause.
JPL kernels are separate data inputs and are never committed to this repository.

## Build and test

```sh
cmake -S . -B build -DOE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Download and verify DE440 through the build:

```sh
cmake --build build --target oe-data
cmake --build build --target oe-test-data
./build/oe-example
```

`oe-data` stores the verified DE440 and Chiron kernels and their manifests in
`data/`. They are ignored by Git. No Python installation and no manually
entered absolute path are required. If Horizons replaces the pinned Chiron
orbit solution, validation fails instead of silently changing results. Horizons
embeds the request date in its comment block, so the manifest records each
download's actual SHA-256 while the downloader pins solution JPL#171, object ID,
and coverage.

## Minimal API example

```c
#include <openephemeris/oe.h>

oe_ephemeris *ephemeris = NULL;
oe_position_result sun;

if (oe_ephemeris_open_default(&ephemeris) == OE_OK &&
    oe_position_at_jd(ephemeris, OE_SUN, 2451545.0, &sun) == OE_OK) {
    /* sun.longitude_deg */
}
oe_ephemeris_close(ephemeris);
```

`oe_ephemeris_open_default()` finds `data/de440.bsp` automatically. Set
`OE_DATA_PATH` only when the data lives elsewhere. Positions and houses are
separate calls, matching the Swiss Ephemeris workflow.

Public angles are degrees, distances are AU, and speeds are per day. Input is
UTC; local timezone/DST conversion is deliberately an application
responsibility. The lower-level time, position, and house functions remain
available for applications that need explicit TT/UT1 control.

## Accuracy evidence

The optional DE440 test checks the apparent geocentric Sun at
2000-01-01 12:00 UTC against a preserved JPL Horizons DE441 value. The current
result is `280.368922564°`, a difference of about `0.048″` from Horizons'
`280.3689092°`, within the stated `0.1″` planetary acceptance threshold.
This single fixture proves the vertical calculation path; it does not replace
the full body/date acceptance matrix required before 1.0.

An independent NAIF SPICE N0067 fixture also checks geometric J2000 ICRF
position and velocity for the Sun, Moon, and every planetary target exposed by
the library. All 60 components agree within `1e-5 km` and `1e-10 km/s`.

The type-21 evaluator is separately checked against a Horizons JPL#171
heliocentric ICRF state vector at J2000. Its six state components agree within
1 mm and `1e-9 km/s`; coverage boundaries and missing-kernel behavior are also
tested.

See [definitions](docs/DEFINITIONS.md), [provenance](docs/PROVENANCE.md), and
[implementation status](docs/STATUS.md), the
[astrological API guide](docs/ASTROLOGY.md), and the
[1.0 release contract](docs/RELEASE_1_0.md) before embedding the preview.

Julian Date is the only public calculation interface. The main entry points
are `oe_position_at_jd()`, `oe_houses_at_jd()` and
`oe_aspect_between_bodies_at_jd()`. See
[`examples/09_julian_date_api.c`](examples/09_julian_date_api.c).
