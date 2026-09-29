# Cube scaling measurements

Shared measurement protocol across machines.

| Machine | Path | Cubes | Rate (/s) | p50 ms | p95 ms |
| --- | --- | ---: | ---: | ---: | ---: |
| i9-9900K / RTX 4070 SUPER | physics-cpu | 50,000 | 19.87 | 50.017 | 54.553 |
| i9-9900K / RTX 4070 SUPER | physics-gpu | 50,000 | 69.26 | 14.268 | 17.917 |
| i9-9900K / RTX 4070 SUPER | direct-cpu | 50,000 | 18.54 | 52.805 | 59.804 |
| i9-9900K / RTX 4070 SUPER | direct-gpu | 50,000 | 68.92 | 14.145 | 18.209 |

Rates for physics paths are completed steps/s; direct-renderer rates are FPS. Missing points are not extrapolated.

- **i9-9900K / RTX 4070 SUPER**: complete; runner checkout d98f2f365429; backend native; stops: `{}`.
