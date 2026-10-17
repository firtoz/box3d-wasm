export const oldHiddenText='\t\tm_textLine = (int)( 3.0f * fontSize );';
export const newHiddenText='\t\t// Reserve the PAUSED row only while that HUD label is present.\n\t\tm_textLine = (int)( ( m_context->pause ? 4.5f : 3.0f ) * fontSize );';
export const oldComparison=`    ImGui::SetNextWindowPos(ImVec2(8.0f, (float)sapp_height() - 8.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::Begin("Comparison view", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
    bool split = both_split_enabled();
    if (ImGui::Checkbox("Split: CPU left / GPU right", &split)) {
        s_context.sample->MouseUp({0, 0}, 0);
        both_set_split(split);
        pointerPane = -1;
    }
    ImGui::End();`;
export const newComparison='    if (s_context.showUI) {\n'+oldComparison.split('\n').map(l=>'    '+l).join('\n')+'\n    }';
