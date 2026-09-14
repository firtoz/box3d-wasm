#pragma once
#include "gfx/text.h"
#include "gfx/renderer.h"
#include <string>
#include <vector>
struct SplitText {
    int x, y;
    Vec4 color;
    std::string text;
};
inline void split_capture_text(std::vector<SplitText>& out, int xOffset) {
    CameraState camera = GetCameraState();
    for (int i = 0; i < GetTextCount(); ++i) {
        const TextEntry* e = GetTextAt(i);
        float x, y;
        if (ResolveTextScreenPos(e, camera.view, camera.proj, camera.viewportW, camera.viewportH, &x, &y))
            out.push_back({int(x) + xOffset, int(y), e->color, e->text});
    }
}
inline void split_publish_text(const std::vector<SplitText>& entries) {
    ResetTextArena();
    for (const auto& e : entries) DrawScreenString(e.x, e.y, e.color, e.text.c_str());
}
