#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Explicit initial-release exclusions only. A false result for an unknown name
// is not a support claim; consult the published API inventory.
bool gpu_b3_native_api_is_unavailable(const char* operation);

// An unavailable operation sets errno=ENOTSUP and a sticky, thread-local name.
// NULL means no unavailable call since this thread's last explicit clear.
// The returned string is static and must not be freed. This diagnostic does not
// clear or replace the engine's sticky world/capacity failure status.
const char* gpu_b3_native_api_last_error(void);
void gpu_b3_native_api_clear_error(void);

// Internal adapter hook. Call only with a static operation-name string.
void gpu_native_api_unavailable(const char* operation);

#ifdef __cplusplus
}
#endif
