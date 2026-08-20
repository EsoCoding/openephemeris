# Astrological API Guide

This document describes the astrology-facing part of the OpenEphemeris C API.
The public header is [include/openephemeris/oe.h](../include/openephemeris/oe.h).
All public functions use the `oe_` prefix and return `oe_status` where a
calculation can fail.

## Coordinate and time conventions

- Angles are degrees. Longitudes are normalized to `[0, 360)`.
- Speeds are degrees per day. Distances are AU and radial speeds are AU/day.
- Planetary positions are apparent geocentric tropical positions.
- `oe_utc_to_jd()` accepts UTC and computes TT and UT1. Pass `NAN` for
  DUT1 when the modeled UT1 value is acceptable.
- `oe_time_from_jd()` is an advanced conversion utility for callers that
  already have separate TT and UT1 values; normal astrology uses one `jd_ut`.
- Planet-position, fixed-star, sidereal, ayanamsa and aspect calculations
  require only `jd_tt`.
- House and complete-chart calculations additionally require `jd_ut1`, because
  they depend on Earth rotation and local sidereal time.
- Timezone and daylight-saving conversion are application responsibilities.
- A result is valid only when its `struct_size` and `abi_version` fields match
  the library ABI. Always check the returned status before reading output.

## Planet positions and houses

Call `oe_position_at_jd()` once per body and `oe_houses_at_jd()` separately for
the observer location. Supported systems are Placidus, Koch, Porphyry, Regiomontanus,
Campanus, Equal, Whole Sign, Alcabitius, Topocentric, Morinus, Meridian,
Vehlow and Equal MC.

For Swiss Ephemeris-style workflows, pass the same `jd_ut` to each call.
`oe_position_at_jd()` requires only `jd_ut`; `oe_houses_at_jd()` and
`oe_house_position_at_jd()` provide the corresponding
lower-level operations. `oe_aspect_between_bodies_at_jd()` calculates both
body positions and then applies the normal aspect/orb logic at that instant;
planetary speeds from the ephemeris are retained for applying/separating
classification.

`oe_aspect_between_bodies_at_jd()` calculates a body pair at a JD.
`oe_part_of_fortune()` uses the standard day/night formula;
the caller determines day or night from the chart houses.

## Fixed stars

`oe_fixed_star_count()` and `oe_fixed_star_at()` enumerate the built-in
catalogue. `oe_fixed_star_find()` performs a case-insensitive lookup by common
name and selected Bayer aliases. Pass the returned immutable record to
`oe_fixed_star_position_at_jd()` to obtain apparent ecliptic-of-date
coordinates.

The current development catalogue is a compact bright-star set. It is not yet
a complete Swiss Ephemeris fixed-star catalogue. A 1.0 release must add the
complete approved catalogue, aliases, source attribution, license review and
independent position fixtures.

## Sidereal and Vedic calculations

`oe_ayanamsa_at_jd()` returns the selected ayanamsa in degrees. The current API
provides Fagan-Bradley, Lahiri, Raman, Krishnamurti, Yukteshwar, True Citra,
True Revati, True Pushya and a user-supplied value.

`oe_sidereal_position_at_jd()` converts a tropical body position to sidereal
longitude and returns zero-based nakshatra index and one-based pada number.
For presentation, display `nakshatra + 1`; the nakshatra width is 13 degrees
20 minutes and each pada is 3 degrees 20 minutes.

`oe_sidereal_houses_at_jd()` computes tropical house geometry and subtracts the
selected ayanamsa from angles and cusps. Rahu and Ketu are represented by
`OE_MEAN_NODE`, `OE_TRUE_NODE`, `OE_MEAN_SOUTH_NODE` and
`OE_TRUE_SOUTH_NODE`.

This is an ephemeris calculation layer. Dashas, divisional charts, yogas and
interpretation are outside the core API.

## Transits and returns

`oe_transit_search()` searches for a body reaching an absolute tropical
longitude from one `start_jd_ut` value. `direction`
must be `+1` or `-1`; `max_days` bounds the search. `oe_return_search()` uses
the same engine with a natal longitude, which is the basic solar/lunar return
operation. The returned `oe_time` is TT/UT1; convert it to a civil
representation in the application if needed.

The current search uses a bounded scan and numerical refinement. It does not
yet expose an orb, aspect type, station policy or fixed-star transit target.
A 1.0 transit API must specify and validate those policies explicitly,
including retrograde and multiple-crossing cases.

## Eclipses

`oe_eclipse_search()` finds the next or previous syzygy from one `jd_ut` and checks lunar
latitude against the eclipse limit. It returns the event time, longitude,
latitude, an approximate magnitude and a totality flag.

The current implementation is a preliminary astrological event search. It is
not a replacement for a contact-time or local-visibility calculation. Solar
and lunar contact times, local circumstances, visibility maps and independent
fixtures are mandatory before calling eclipse support 1.0 quality.

## Error handling

The most common statuses are:

- `OE_ERR_INVALID_ARGUMENT`: invalid pointer, enum, date, location or search
  direction.
- `OE_ERR_NO_COVERAGE`: the active kernel does not cover the requested date or
  the bounded search found no event.
- `OE_ERR_HOUSES_UNDEFINED`: the requested house geometry is undefined,
  commonly for circumpolar Placidus cases.
- `OE_ERR_NUMERIC`: a mathematical transformation could not produce a valid
  result.

Applications should not silently substitute another ayanamsa, house system,
kernel or event algorithm after an error.

## Examples

- `examples/06_fixed_stars.c` demonstrates catalogue enumeration and lookup.
- `examples/07_sidereal_vedic.c` demonstrates ayanamsa, sidereal positions,
  nakshatra/pada and sidereal houses.
- `examples/08_transits_returns_eclipses.c` demonstrates transits, solar/lunar
  returns and eclipse search.
- `examples/09_julian_date_api.c` demonstrates the standard JD positions,
  houses and aspects API.

Build and run the examples with:

```sh
cmake -S . -B build -DOE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
cmake --build build --target oe-data
./build/oe-example-06_fixed_stars
./build/oe-example-07_sidereal_vedic
./build/oe-example-08_transits_returns_eclipses
```
