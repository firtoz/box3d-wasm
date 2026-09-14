#pragma once
// The pane that accepted a button-down owns that gesture until release. Do not
// wrap a drag at the divider: coordinates outside its pane remain outside.
inline float split_pointer_x(float x, int windowWidth, int capturedPane = -1) {
    const int width = windowWidth / 2;
    const int pane = capturedPane >= 0 ? capturedPane : (x >= width ? 1 : 0);
    return x - pane * width;
}
