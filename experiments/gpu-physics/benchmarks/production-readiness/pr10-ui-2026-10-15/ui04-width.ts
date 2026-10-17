/** Exact source replacement used by the generated native sample, and its audit. */
export const oldSection='\t\t\tImGui::TableSetupColumn( "section", ImGuiTableColumnFlags_WidthFixed, 8.0f * fontSize );';
export const newSection=`\t\t\t// Include the same indentation and arrow spacing used to draw every row.
\t\t\t// Measure collapsed/zero rows too, so expanding a subtree keeps labels visible.
\t\t\tfloat sectionWidth = ImGui::CalcTextSize( "section" ).x;
\t\t\tconst float labelSpacing = ImGui::GetTreeNodeToLabelSpacing();
\t\t\tfor ( const RowDef& row : rows )
\t\t\t{
\t\t\t\tfloat width = row.indent * fontSize + labelSpacing + ImGui::CalcTextSize( row.name ).x;
\t\t\t\tsectionWidth = b3MaxFloat( sectionWidth, width );
\t\t\t}
\t\t\tImGui::TableSetupColumn( "section", ImGuiTableColumnFlags_WidthFixed, sectionWidth );`;
