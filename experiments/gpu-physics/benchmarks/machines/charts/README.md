# Cube scaling measurements

Shared measurement protocol across machines.

| Machine | Path | Cubes | Rate (/s) | p50 ms | p95 ms |
| --- | --- | ---: | ---: | ---: | ---: |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 100 | 6489.52 | 0.152 | 0.176 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 1,000 | 1322.63 | 0.748 | 0.819 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 5,000 | 310.41 | 3.166 | 3.794 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 10,000 | 132.12 | 7.479 | 8.532 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 25,000 | 44.59 | 22.218 | 24.741 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 50,000 | 19.87 | 50.017 | 54.553 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 100,000 | 9.33 | 107.006 | 114.953 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 150,000 | 5.75 | 173.023 | 186.694 |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 200,000 | 4.19 | 233.001 | 256.339 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 100 | 496.38 | 1.937 | 2.355 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 1,000 | 333.12 | 2.782 | 4.174 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 5,000 | 266.66 | 3.567 | 4.978 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 10,000 | 194.47 | 4.848 | 7.339 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 25,000 | 123.42 | 8.017 | 10.050 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 50,000 | 69.26 | 14.268 | 17.917 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 100,000 | 36.30 | 26.871 | 34.092 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 150,000 | 24.80 | 38.954 | 50.062 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 200,000 | 18.84 | 51.652 | 64.583 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 100 | 3446.99 | 0.287 | 0.348 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 1,000 | 1058.66 | 0.937 | 1.050 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 5,000 | 265.46 | 3.713 | 4.193 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 10,000 | 118.50 | 8.380 | 9.265 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 25,000 | 41.82 | 23.983 | 25.993 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 50,000 | 18.54 | 52.805 | 59.804 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 100,000 | 8.77 | 113.900 | 123.151 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 150,000 | 5.22 | 190.879 | 203.935 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 200,000 | 3.84 | 259.085 | 276.256 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 100 | 511.85 | 1.874 | 2.425 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 1,000 | 339.15 | 2.699 | 4.484 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 5,000 | 262.98 | 3.613 | 5.158 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 10,000 | 191.69 | 4.884 | 7.440 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 25,000 | 124.21 | 7.750 | 10.076 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 50,000 | 68.92 | 14.145 | 18.209 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 100,000 | 35.90 | 26.812 | 35.184 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 150,000 | 24.48 | 39.179 | 51.206 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 200,000 | 18.61 | 51.787 | 66.518 |

Rates for physics paths are completed steps/s; direct-renderer rates are FPS. Missing points are not extrapolated.

- **i9-9900K / RTX 4070 SUPER**: complete; runner checkout 8f64b2c581a9, d98f2f365429; backend native; stops: `{}`.
