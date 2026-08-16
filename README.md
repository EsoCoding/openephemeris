# OpenEphemeris

OpenEphemeris is a clean-room, permissively licensed C11 calculation library
for high-precision tropical astrology. It reads official JPL SPK kernels
directly and exposes a small, versioned `oe_` C ABI. The core never downloads
data, uses global mutable state, silently changes ephemeris, or substitutes a
different house system.

> Current release status: **0.1.0 development preview**, not 1.0. Planetary
> DE440 calculations and Placidus are operational. Horizons type-21 support,
> required for Chiron, remains a documented 1.0 gate. See
> [docs/STATUS.md](docs/STATUS.md).

## Implemented scope

- apparent geocentric tropical longitude, latitude, distance, and speeds;
- Sun, Moon, Mercury through Pluto using a JPL DE440 kernel;
- mean/true lunar nodes and mean/true Black Moon Lilith;
- Placidus cusps, Ascendant, MC, ARMC, and geometric house position;
- explicit UTC/TT/UT1 handling and modeled-time quality flags;
- endian-aware, bounds-checked DAF/SPK type 2 and 3 input;
- static and shared libraries on Linux and Windows-oriented CMake builds.

The code is MIT licensed. The vendored ERFA 2.0.1 dependency is BSD-3-Clause.
JPL kernels are separate data inputs and are never committed to this repository.

## Build and test

```sh
cmake -S . -B build -DOE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Fetch and verify DE440 with the optional, non-runtime Python tool:

```sh
python3 tools/oe_data.py --output data
OE_TEST_KERNEL=/absolute/path/to/data/de440.bsp \
  ctest --test-dir build --output-on-failure
./build/oe-example data/de440.bsp
```

`tools/oe_data.py --chiron` can also preserve a reproducible Horizons Chiron
kernel and manifest, but the runtime rejects its type-21 segment until that
reader passes the independent validation gate.

## Minimal API example

```c
#include <openephemeris/oe.h>

oe_ephemeris *ephemeris = NULL;
oe_time time;
oe_position_result sun;

if (oe_ephemeris_open("de440.bsp", NULL, &ephemeris) == OE_OK &&
    oe_time_from_utc(2000, 1, 1, 12, 0, 0.0, 0.0, &time) == OE_OK &&
    oe_position(ephemeris, OE_SUN, &time, &sun) == OE_OK) {
    /* sun.longitude_deg and sun.longitude_speed_deg_per_day */
}
oe_ephemeris_close(ephemeris);
```

Public angles are degrees, distances are AU, and speeds are per day. Local
timezone/DST conversion is deliberately an application responsibility.

## Accuracy evidence

The optional DE440 test checks the apparent geocentric Sun at
2000-01-01 12:00 UTC against a preserved JPL Horizons DE441 value. The current
result is `280.368922564°`, a difference of about `0.048″` from Horizons'
`280.3689092°`, within the stated `0.1″` planetary acceptance threshold.
This single fixture proves the vertical calculation path; it does not replace
the full body/date acceptance matrix required before 1.0.

See [definitions](docs/DEFINITIONS.md), [provenance](docs/PROVENANCE.md), and
[implementation status](docs/STATUS.md) before embedding the preview.
