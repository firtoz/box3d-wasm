#pragma once
#include <stdbool.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

// Explicit initial-release exclusions only. A false result for an unknown name
// is not a support claim; consult the published API inventory.
bool gpu_b3_native_api_is_unavailable(const char* operation);

// An unavailable operation sets errno=ENOTSUP and a sticky, thread-local name.
// Invalid/busy/failed diagnostic queries set EINVAL/EBUSY/EIO respectively.
// NULL means no API error since this thread's last explicit clear.
// The returned string is static and must not be freed. This diagnostic does not
// clear or replace the engine's sticky world/capacity failure status.
const char* gpu_b3_native_api_last_error(void);
void gpu_b3_native_api_clear_error(void);

// Internal adapter hook. Call only with a static operation-name string.
void gpu_native_api_unavailable(const char* operation);
void gpu_native_api_error(const char* operation, int error_code);

// Shared Rust/C thread-local error state and registry-ownership precondition.
bool gpu_b3_registry_access_allowed(void);
const char* gpu_b3_native_api_error_name(void);
void gpu_b3_native_api_reset_error(void);
void gpu_b3_native_api_record_error(const char* operation, int error_code);
static inline bool gpu_native_api_check_access(const char* operation)
{
    if (gpu_b3_registry_access_allowed()) return true;
    gpu_b3_native_api_record_error(operation, EBUSY);
    errno = EBUSY;
    return false;
}
#define GPU_NATIVE_ACCESS_OR_RETURN() do { if (!gpu_native_api_check_access(__func__)) return; } while (0)
#define GPU_NATIVE_ACCESS_OR_RETURN_ZERO(type) do { if (!gpu_native_api_check_access(__func__)) return (type){0}; } while (0)

#ifdef __cplusplus
}
#endif
