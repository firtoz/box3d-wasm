#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Immutable for the renderer lifetime: callbacks retain pointers into the pool.
int sample_renderer_capacity(void);
void sample_renderer_release_adapter(void);
uint64_t sample_renderer_instance_count(void);
#ifdef __cplusplus
}
#endif
