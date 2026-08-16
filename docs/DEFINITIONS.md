# Definitions and units

- Public angular results are tropical degrees in `[0, 360)`; angular speeds are degrees/day.
- Distances are astronomical units and radial speeds are AU/day.
- Planetary results are apparent geocentric directions: iterative light time,
  solar deflection, annual aberration, IAU 2006 ecliptic-of-date, and IAU 2000A nutation longitude.
- JPL Horizons' `ObsEcLon` compatibility column uses the older IAU76/80
  ecliptic-of-date. It is therefore not an interchangeable sub-arcsecond
  reference away from J2000; raw ICRF statevectors are used to validate the
  SPK reader independently of this frame-definition difference.
- The mean node uses the conventional mean lunar ascending-node polynomial.
- The true node is the ascending intersection of the instantaneous geocentric
  lunar orbital plane and IAU 2006 ecliptic of date.
- Mean Lilith is mean lunar perigee plus 180 degrees. True Lilith is opposite
  the instantaneous eccentricity vector derived from the Earth-Moon state.
- Placidus divides semi-diurnal and semi-nocturnal arcs into thirds. Undefined
  circumpolar geometry is an error and never triggers a fallback.
- Civil timezone and DST resolution are outside this library. `oe_time_from_utc`
  interprets the supplied fields as UTC; direct TT/UT1 input is authoritative.
