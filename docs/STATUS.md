# Implementation status

## Implemented and tested

- C11 static/shared library, stable versioned structs and `oe_` symbols.
- Read-only, bounds-checked, endian-aware DAF/SPK type 2 and 3 reader.
- Recursive target/center state resolution without silent fallback.
- DE440 apparent geocentric positions and finite-difference speeds.
- Mean/true lunar nodes and mean/true Lilith.
- Placidus cusps, Ascendant, MC, ARMC, and geometric mundane position.
- UTC/TT/UT1 conversion with explicit modeled-time quality flags.
- Linux GCC warnings-as-errors build and an optional live DE440 fixture.

## Required before 1.0

- Independent SPK type 21 evaluator for Horizons Chiron kernels. Chiron returns
  `OE_ERR_UNSUPPORTED_KERNEL` until this exists; it never returns an approximation.
- Broad Horizons fixtures for every body and epoch in the acceptance matrix.
- Independent Placidus fixtures and high-latitude boundary audit.
- Windows MSVC CI execution and ABI compatibility baseline.
- Sanitizer/fuzzer corpus and external legal provenance review.

The project intentionally reports version 0.1.0 until these gates are closed.
