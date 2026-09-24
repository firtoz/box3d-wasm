#include "sokol_capacity.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned long long read_count(const char* name)
{
    const char* value = getenv(name);
    if (!value || !*value) return 0;
    char* end;
    errno = 0;
    unsigned long long n = strtoull(value, &end, 10);
    if (errno || *end || value[0] == '-' || n == 0 || n > INT_MAX)
    {
        fprintf(stderr, "Invalid positive renderer count %s=%s\n", name, value);
        exit(EXIT_FAILURE);
    }
    return n;
}

int sample_renderer_capacity(void)
{
    static int capacity;
    if (capacity) return capacity;
    unsigned long long requested = read_count("GPU_SAMPLES_SHAPE_CAPACITY");
    unsigned long long cubes = read_count("GPU_BENCH_CUBES");
    unsigned long long required = cubes ? cubes + 1 : 0;
#ifdef BOTH_SAMPLES
    required *= 2;
#endif
    if (requested && requested < required)
    {
        fputs("Renderer reservation is smaller than the benchmark scene\n", stderr);
        exit(EXIT_FAILURE);
    }
    unsigned long long count = requested ? requested : (required > 65536 ? required : 65536);
    if (count > INT_MAX)
    {
        fputs("Renderer reservation exceeds signed index range\n", stderr);
        exit(EXIT_FAILURE);
    }
    capacity = (int)count;
    fprintf(stderr, "sokol-capacity: %d debug shapes and opaque instances per stream\n", capacity);
    return capacity;
}
