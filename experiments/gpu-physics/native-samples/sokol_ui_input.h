#pragma once

#include "sokol_app.h"
#include "imgui.h"
#include "imgui_internal.h"

// HandleEvent has already queued this event for ImGui. Its capture flag still
// describes the previous UI frame, so a move and wheel in one event batch can
// scroll the panel and zoom the camera together. Query the wheel's own position.
inline bool gpu_ui_capture_event(const sapp_event* e, bool captured)
{
    const unsigned buttons = SAPP_MODIFIER_LMB | SAPP_MODIFIER_RMB | SAPP_MODIFIER_MMB;
    if (e->type != SAPP_EVENTTYPE_MOUSE_SCROLL || (e->modifiers & buttons) ||
        !ImGui::GetCurrentContext())
        return captured;

    // Keep UI gestures and popup/modal ownership even outside their bounds.
    if (ImGui::IsAnyItemActive() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
        return true;

    ImGuiWindow* hovered = nullptr;
    // The native host uses physical framebuffer pixels for both Sokol events
    // and ImGui (StartUIFrame sets dpi_scale to 1.0).
    ImGui::FindHoveredWindowEx(ImVec2(e->mouse_x, e->mouse_y), false, &hovered, nullptr);
    return hovered != nullptr;
}
