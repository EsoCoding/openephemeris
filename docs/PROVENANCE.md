# Algorithm and data provenance

No Swiss Ephemeris source, table, constant, or implementation was consulted or
copied. Swiss output may be used later only as an optional black-box comparison.

| Area | Source |
|---|---|
| DAF/SPK container and type 2/3/21 records | NASA NAIF, *DAF Required Reading* and *SPK Required Reading* |
| Planetary and lunar state vectors | NASA/JPL DE440 SPK |
| Chiron state vectors | NASA/JPL Horizons, solution JPL#171 dated 2026-06-05, SPK ID 20002060 |
| Precession, nutation, ecliptic frame, sidereal time, aberration and light deflection | ERFA 2.0.1 / IAU SOFA models |
| Calendar and historical Delta-T estimates | Espenak/Meeus polynomial family, with modeled quality flags |
| Mean lunar node and perigee | Published lunar fundamental-argument polynomials |
| Osculating node and apogee | State-vector orbital geometry, documented in `docs/DEFINITIONS.md` |
| Placidus cusps and mundane positions | Independent spherical-geometry implementation of semi-diurnal/nocturnal arc trisection |

The type-21 implementation was written in this repository from the public SPK
format description. No NAIF source is shipped or linked. Its output is checked
against the official NAIF SPICE N0067 reader. Horizons kernels are retained
unchanged; their non-deterministic request metadata means the manifest records
the downloaded file hash while object `20002060`, coverage, and solution
`JPL#171` are the stable pin.

References:

- https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/daf.html
- https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/spk.html
- https://github.com/liberfa/erfa
- https://ssd.jpl.nasa.gov/doc/de440_de441.html
- https://ssd-api.jpl.nasa.gov/doc/horizons.html
