# 1.0 Release Contract

Version 1.0 is an astrology-first release. The astronomical kernel is the
source of planetary positions; the public product contract is the reliable
calculation of natal-chart and predictive astrology primitives.

## Required public capabilities

1. Tropical planetary positions, speeds, nodes, Lilith and Chiron.
2. All documented house systems, angles and geometric house positions.
3. Signs, aspects, Part of Fortune and chart-level calculations.
4. Complete fixed-star catalogue with stable names, aliases and provenance.
5. Sidereal zodiac and all promised ayanamsa modes with reference fixtures.
6. Sidereal houses, Rahu/Ketu, nakshatra and pada calculations.
7. Transit searches for supported bodies and fixed stars, including aspect
   targets, direction, orb and retrograde/multiple-crossing behavior.
8. Solar and lunar returns using the same time and location contract as natal
   charts.
9. Solar and lunar eclipse events with validated maximum and contact times.
10. Explicit UTC, TT, UT1 and topocentric behavior with quality flags.

## Quality gates

- Every supported body and point has fixtures across historical, modern and
  future dates within its documented kernel coverage.
- Tropical and sidereal positions are compared against an independent
  Swiss Ephemeris and/or Horizons acceptance matrix.
- Fixed-star names, aliases, proper motion and catalog provenance are reviewed
  for completeness and licensing.
- Houses have independent fixtures at ordinary, high-latitude and boundary
  locations; undefined systems return errors rather than fallbacks.
- Transit and return searches test forward/backward searches, exact starts,
  retrograde motion, ingress boundaries and multiple crossings.
- Eclipse tests cover solar/lunar type, maximum, contact ordering and no-event
  intervals.
- Linux GCC, Windows MSVC, warnings-as-errors, ASan/UBSan and fuzzing pass.
- ABI structs are size/version checked and documented examples compile from a
  clean build.
- README, API guide, provenance, definitions and status describe the same
  behavior as the implementation.

## Explicitly outside 1.0

Occultations, heliacal calculations, rise/set, Gauquelin sectors, full
Swiss-Ephemeris symbol-for-symbol API compatibility, dashas, divisional charts,
yogas and interpretive astrology are separate follow-up scopes unless they are
promoted by a later release decision.

## Current state

The repository currently reports version 0.2.0. The new public examples and
the first fixed-star, sidereal, transit, return and eclipse APIs are available,
but the quality gates above remain open. The project must not label this code
1.0 until the independent validation and catalog/provenance work is complete.
