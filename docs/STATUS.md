# Implementation status

## Implemented and tested

- C11 static/shared library, stable versioned structs and `oe_` symbols.
- Read-only, bounds-checked, endian-aware DAF/SPK type 2, 3, and 21 reader.
- Recursive target/center state resolution without silent fallback.
- DE440 apparent geocentric positions and finite-difference speeds.
- Mean/true lunar nodes and mean/true Lilith.
- Chiron from a checksum-pinned Horizons JPL#171 kernel for 1800--2200.
- Placidus cusps, Ascendant, MC, ARMC, and geometric mundane position.
- UTC/TT/UT1 conversion with explicit modeled-time quality flags.
- Simple `oe_ephemeris_open_default()` data discovery and one-call
  `oe_chart_from_utc()` calculation for ordinary application use.
- Fixed-star lookup and apparent ecliptic positions for the built-in bright-star
  catalogue.
- Ayanamsa, sidereal positions/houses, and nakshatra/pada calculation.
- Bounded transit/return search and a preliminary eclipse search.
- Linux GCC warnings-as-errors and ASan/UBSan builds.
- Independent NAIF SPICE J2000 state fixtures for every DE440-backed public
  body, plus the independent Horizons Chiron state fixture.

## Required before 1.0

- Broad Horizons fixtures for every body and epoch in the acceptance matrix.
- Expanded fixed-star catalogue with documented Swiss Ephemeris-compatible
  source, aliases and licensing for every entry.
- Swiss Ephemeris validation for every ayanamsa mode and independent fixtures
  for transits, returns and solar/lunar eclipses.
- Independent Placidus fixtures and high-latitude boundary audit.
- Windows MSVC CI execution and ABI compatibility baseline.
- Sanitizer/fuzzer corpus and external legal provenance review.

The project intentionally reports a development version until these gates are
closed; the current ABI reports version 0.2.0.
