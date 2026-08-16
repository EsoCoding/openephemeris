# Algorithm and data provenance

No Swiss Ephemeris source, table, constant, or implementation was consulted or
copied. Swiss output may be used later only as an optional black-box comparison.

| Area | Source |
|---|---|
| DAF/SPK container and type 2/3 records | NASA NAIF, *DAF Required Reading* and *SPK Required Reading* |
| Planetary and lunar state vectors | NASA/JPL DE440 SPK |
| Precession, nutation, ecliptic frame, sidereal time, aberration and light deflection | ERFA 2.0.1 / IAU SOFA models |
| Calendar and historical Delta-T estimates | Espenak/Meeus polynomial family, with modeled quality flags |
| Mean lunar node and perigee | Published lunar fundamental-argument polynomials |
| Osculating node and apogee | State-vector orbital geometry, documented in `docs/DEFINITIONS.md` |
| Placidus cusps and mundane positions | Independent spherical-geometry implementation of semi-diurnal/nocturnal arc trisection |

References:

- https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/daf.html
- https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/spk.html
- https://github.com/liberfa/erfa
- https://ssd.jpl.nasa.gov/doc/de440_de441.html
