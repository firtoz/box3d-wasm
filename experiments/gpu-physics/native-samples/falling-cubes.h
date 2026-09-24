// Falling-cubes v1. Binary-fraction spacing makes the Rust/C setup exact.
#pragma once
#include <cstdint>
inline uint32_t fallingColumns(uint32_t count) { return (count + 9) / 10; }
inline uint32_t fallingWidth(uint32_t count) {
    uint32_t w = 1; while (w * w < fallingColumns(count)) ++w; return w;
}
inline float fallingGround(uint32_t count) { return 1.25f * fallingWidth(count) + 12.0f; }
inline b3Pos fallingPosition(uint32_t count, uint32_t i) {
    uint32_t columns = fallingColumns(count), w = fallingWidth(count);
    uint32_t layer = i / columns, column = i % columns;
    float offset = (layer % 2) ? 0.125f : -0.125f;
    float center = 0.625f * (w - 1);
    return {1.25f * (column % w) - center + offset, 6.0f + 1.25f * layer,
            1.25f * (column / w) - center - offset};
}
