"""Native split comparison hooks, applied after the timing/loading hooks."""
def replace_once(text, old, new):
    if text.count(old) != 1:
        raise RuntimeError(f'split hook drift: {old!r}')
    return text.replace(old, new)

INCLUDE = '\n#if defined(BOTH_SAMPLES)\n#include "../c_abi/both_view.h"\n#include "sokol_split_stream.h"\n#include "sokol_split_input.h"\n#include "sokol_split_text.h"\n#endif\n'

def main(text):
    text = INCLUDE + '#if defined(BOTH_SAMPLES)\n#include \"imgui.h\"\n#endif\n' + text
    text = text.replace('#include "sokol_split_text.h"', '#include "sokol_split_text.h"\nstatic int pointerPane = -1;\nstatic float splitMouseX = -1, splitMouseY = -1;\nstatic bool splitMouseInside = false;')
    text = replace_once(text, '\tcamera.Update( dt, W, H );', '''#if defined(BOTH_SAMPLES)
    const bool splitActive = both_split_enabled() && W >= 2;
    camera.Update(dt, splitActive ? W / 2 : W, H);
    both_begin_frame();
    if (both_split_enabled()) {
        PickRay ray = camera.BuildPickRay(s_context.mouseX, s_context.mouseY);
        both_pointer_move(ray.origin, ray.translation);
    }
#else
    camera.Update(dt, W, H);
#endif''')
    text = replace_once(text, '\tRenderFrame( &sc, &fi );', '''#if defined(BOTH_SAMPLES)
    if (splitActive) {
        split_stream_view(0);
        split_render_viewport(0, W / 2);
        RenderFrame(&sc, &fi);
        std::vector<SplitText> labels;
        split_capture_text(labels, 0);
        ResetFrameArena();
        split_stream_view(1);
        both_draw_second();
        s_context.sample->Render();
        split_render_viewport(W / 2, W / 2);
        RenderFrame(&sc, &fi);
        split_capture_text(labels, W / 2);
        split_publish_text(labels);
        split_stream_view(0);
        split_render_viewport(0, 0);
    } else {
        split_stream_view(0);
        split_render_viewport(0, 0);
        RenderFrame(&sc, &fi);
    }
#else
    RenderFrame(&sc, &fi);
#endif''')
    # UI sees physical window coordinates; only the sample receives pane-local coordinates.
    text = replace_once(text, '\tif ( uiCaptured )\n\t{\n\t\treturn;\n\t}', '''#if defined(BOTH_SAMPLES)
    if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE || e->type == SAPP_EVENTTYPE_MOUSE_DOWN || e->type == SAPP_EVENTTYPE_MOUSE_UP || e->type == SAPP_EVENTTYPE_MOUSE_ENTER) {
        splitMouseX = e->mouse_x; splitMouseY = e->mouse_y; splitMouseInside = true;
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_LEAVE || e->type == SAPP_EVENTTYPE_UNFOCUSED) splitMouseInside = false;
    if (releaseOrUnfocus) {
        both_pointer_up();
        pointerPane = -1;
    }
#endif
    if (uiCaptured) return;
#if defined(BOTH_SAMPLES)
    sapp_event localEvent = *e;
    if (both_split_enabled() && sapp_width() >= 2) {
        int paneWidth = sapp_width() / 2;
        if (e->type == SAPP_EVENTTYPE_MOUSE_DOWN && pointerPane < 0) pointerPane = e->mouse_x >= paneWidth ? 1 : 0;
        localEvent.mouse_x = split_pointer_x(e->mouse_x, sapp_width(), pointerPane);
        e = &localEvent;
    }
#endif''')
    text = replace_once(text, '\tDrawUI( &s_context );', '''#if defined(BOTH_SAMPLES)
    ImGui::SetNextWindowPos(ImVec2(8.0f, (float)sapp_height() - 8.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::Begin("Comparison view", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
    bool split = both_split_enabled();
    if (ImGui::Checkbox("Split: CPU left / GPU right", &split)) {
        s_context.sample->MouseUp({0, 0}, 0);
        both_set_split(split);
        pointerPane = -1;
    }
    ImGui::End();
#endif
    // UI layout uses the whole window; projection/picking keeps the pane size.
#if defined(BOTH_SAMPLES)
    int paneWidth = s_context.camera.m_width;
    s_context.camera.m_width = sapp_width();
    if (both_split_enabled()) {
        auto* fg = ImGui::GetForegroundDrawList();
        fg->AddLine(ImVec2(sapp_width()/2, 22), ImVec2(sapp_width()/2, sapp_height()), IM_COL32(160,160,170,255));
        fg->AddText(ImVec2(sapp_width()/4 - 12, 28), IM_COL32(255,190,70,255), "CPU");
        fg->AddText(ImVec2(3*sapp_width()/4 - 12, 28), IM_COL32(130,180,255,255), "GPU");
        if (splitMouseInside && (!ImGui::GetIO().WantCaptureMouse || pointerPane >= 0)) {
            const int width = sapp_width() / 2;
            const int sourcePane = pointerPane >= 0 ? pointerPane : (splitMouseX >= width ? 1 : 0);
            const int peer = 1 - sourcePane;
            const float x = split_pointer_x(splitMouseX, sapp_width(), sourcePane) + peer * width;
            const float y = splitMouseY;
            const ImU32 color = peer ? IM_COL32(130,180,255,255) : IM_COL32(255,190,70,255);
            fg->PushClipRect(ImVec2(peer * width, 0), ImVec2((peer + 1) * width, sapp_height()), true);
            fg->AddLine(ImVec2(x-8,y), ImVec2(x+8,y), IM_COL32(0,0,0,220), 4);
            fg->AddLine(ImVec2(x,y-8), ImVec2(x,y+8), IM_COL32(0,0,0,220), 4);
            fg->AddLine(ImVec2(x-7,y), ImVec2(x+7,y), color, 2);
            fg->AddLine(ImVec2(x,y-7), ImVec2(x,y+7), color, 2);
            if (pointerPane >= 0) fg->AddCircle(ImVec2(x,y), 11, color, 16, 2);
            fg->PopClipRect();
        }
    }
#endif
    DrawUI( &s_context );
#if defined(BOTH_SAMPLES)
    s_context.camera.m_width = paneWidth;
#endif''')
    return text

def sample(text):
    text = INCLUDE + text
    anchor = '\tif ( button == 0 && modifiers == 0 )'
    text = replace_once(text, anchor, '''#if defined(BOTH_SAMPLES)
    if (both_split_enabled() && button == 0 && (modifiers == 0 || modifiers == MOD_CTRL)) {
        PickRay ray = m_camera->BuildPickRay(p.x, p.y);
        both_pointer_down(m_worldId, ray.origin, ray.translation, modifiers == MOD_CTRL, m_mouseForceScale);
        return;
    }
#endif
''' + anchor)
    text = replace_once(text, 'void Sample::MouseUp( b3Vec2 p, int button )\n{', '''void Sample::MouseUp( b3Vec2 p, int button )
{
#if defined(BOTH_SAMPLES)
    both_pointer_up();
#endif''')
    text = replace_once(text, '\tif ( B3_IS_NON_NULL( m_mouseJointId ) )\n\t{\n\t\tm_mousePoint', '''#if defined(BOTH_SAMPLES)
    if (both_split_enabled()) both_pointer_move(pickRay.origin, pickRay.translation);
#endif
    if ( B3_IS_NON_NULL( m_mouseJointId ) )
    {
        m_mousePoint''')
    return text

def joint(text):
    # Door's Ctrl-click is an impulse tool rather than the base mouse joint.
    anchor = '\t\t\tb3RayResult result = b3World_CastRayClosest( m_worldId, pickRay.origin, pickRay.translation, b3DefaultQueryFilter() );'
    return INCLUDE + replace_once(text, anchor, '''#if defined(BOTH_SAMPLES)
            if (both_split_enabled()) {
                both_pointer_impulse(m_worldId, pickRay.origin, pickRay.translation,
                    m_magnitude * b3Normalize(pickRay.translation));
                return;
            }
#endif
''' + anchor)
