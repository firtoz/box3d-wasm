# Cube scaling measurements

Shared measurement protocol across machines.

| Machine | Path | Cubes | Rate (/s) | p50 ms | p95 ms |
| --- | --- | ---: | ---: | ---: | ---: |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 100 | 6414.56 | 0.154 | 0.176 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 1,000 | 1297.93 | 0.761 | 0.815 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 5,000 | 311.41 | 3.166 | 3.683 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 10,000 | 130.08 | 7.614 | 8.518 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 25,000 | 44.49 | 22.299 | 24.222 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 50,000 | 20.03 | 49.794 | 53.035 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 100,000 | 9.23 | 108.045 | 117.384 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 150,000 | 5.73 | 172.595 | 186.107 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 200,000 | 4.20 | 236.531 | 255.807 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 100 | 937.37 | 0.995 | 1.422 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 1,000 | 272.41 | 3.506 | 5.263 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 5,000 | 74.17 | 13.844 | 15.730 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 10,000 | 38.59 | 26.589 | 29.851 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 25,000 | 16.54 | 63.389 | 65.747 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 50,000 | 7.82 | 135.774 | 139.031 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 100 | 3330.95 | 0.296 | 0.330 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 1,000 | 1072.99 | 0.921 | 1.067 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 5,000 | 267.59 | 3.705 | 4.159 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 10,000 | 117.03 | 8.540 | 9.602 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 25,000 | 40.52 | 24.497 | 27.074 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 50,000 | 18.73 | 53.329 | 57.272 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 100 | 928.06 | 0.964 | 1.554 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 1,000 | 265.61 | 3.553 | 5.339 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 5,000 | 73.16 | 13.886 | 16.121 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 10,000 | 38.24 | 26.597 | 30.082 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 25,000 | 16.37 | 63.518 | 65.726 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 50,000 | 7.69 | 137.017 | 140.026 |

Rates for physics paths are completed steps/s; direct-renderer rates are FPS. Missing points are not extrapolated.

- **i9-9900K / RTX 4070 SUPER**: partial; runner checkout d98f2f365429; backend native; stops: `{}`.
