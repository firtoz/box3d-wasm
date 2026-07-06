#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

typedef uint32_t ImU32;
typedef int ImGuiID;
typedef int ImGuiSliderFlags;
enum { ImGuiSliderFlags_None = 0, ImGuiSliderFlags_AlwaysClamp = 1 << 0, ImGuiSliderFlags_Logarithmic = 1 << 2, ImGuiSliderFlags_NoRoundToFormat = 1 << 3, ImGuiSliderFlags_NoInput = 1 << 4 }; 
typedef int ImGuiSelectableFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiCond;
typedef int ImGuiHoveredFlags;
typedef int ImGuiCol;
typedef int ImGuiInputTextFlags;
typedef int ImGuiColorEditFlags;
typedef int ImGuiWindowFlags;
typedef int ImGuiChildFlags;
typedef int ImGuiTabItemFlags;
typedef int ImGuiPopupFlags;

struct ImVec2 { float x, y; ImVec2() : x(0), y(0) {} ImVec2(float _x, float _y) : x(_x), y(_y) {} };
struct ImVec4 { float x, y, z, w; ImVec4() : x(0), y(0), z(0), w(0) {} ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {} };

#define IM_COL32(r,g,b,a) (((ImU32)(a)<<24) | ((ImU32)(b)<<16) | ((ImU32)(g)<<8) | ((ImU32)(r)<<0))
#define IM_COL32_BLACK IM_COL32(0,0,0,255)
#define IM_COL32_WHITE IM_COL32(255,255,255,255)

enum ImGuiCond_ { ImGuiCond_None = 0, ImGuiCond_Always = 1, ImGuiCond_Once = 2, ImGuiCond_FirstUseEver = 4, ImGuiCond_Appearing = 8 };
enum ImGuiCol_ { ImGuiCol_Text = 0, ImGuiCol_WindowBg = 8, ImGuiCol_Button = 23, ImGuiCol_ButtonHovered = 24, ImGuiCol_ButtonActive = 25 };
enum ImGuiWindowFlags_ { ImGuiWindowFlags_None = 0, ImGuiWindowFlags_NoTitleBar = 1, ImGuiWindowFlags_NoResize = 4, ImGuiWindowFlags_NoMove = 8, ImGuiWindowFlags_NoCollapse = 16, ImGuiWindowFlags_AlwaysAutoResize = 64, ImGuiWindowFlags_NoBackground = 128, ImGuiWindowFlags_NoSavedSettings = 256, ImGuiWindowFlags_MenuBar = 512, ImGuiWindowFlags_HorizontalScrollbar = 1024, ImGuiWindowFlags_NoFocusOnAppearing = 2048, ImGuiWindowFlags_NoBringToFrontOnFocus = 4096, ImGuiWindowFlags_AlwaysVerticalScrollbar = 8192, ImGuiWindowFlags_AlwaysHorizontalScrollbar = 16384, ImGuiWindowFlags_AlwaysUseWindowPadding = 32768, ImGuiWindowFlags_NoNavInputs = 65536, ImGuiWindowFlags_NoNavFocus = 131072, ImGuiWindowFlags_UnsavedDocument = 262144, ImGuiWindowFlags_NoDocking = 524288, ImGuiWindowFlags_NoNav = 262144 | 131072, ImGuiWindowFlags_NoDecoration = 1 | 4 | 8 | 16, ImGuiWindowFlags_NoInputs = 65536 | 131072 | 1 | 4 | 8, ImGuiWindowFlags_NoScrollbar = 2048 };
enum ImGuiTreeNodeFlags_ { ImGuiTreeNodeFlags_None = 0, ImGuiTreeNodeFlags_CollapsingHeader = 1 << 0, ImGuiTreeNodeFlags_DefaultOpen = 1 << 1, ImGuiTreeNodeFlags_OpenOnDoubleClick = 1 << 5, ImGuiTreeNodeFlags_OpenOnArrow = 1 << 4, ImGuiTreeNodeFlags_Leaf = 1 << 8, ImGuiTreeNodeFlags_Bullet = 1 << 9, ImGuiTreeNodeFlags_Framed = 1 << 10, ImGuiTreeNodeFlags_SpanAvailWidth = 1 << 11, ImGuiTreeNodeFlags_SpanFullWidth = 1 << 12, ImGuiTreeNodeFlags_Selected = 1 << 13, ImGuiTreeNodeFlags_NoTreePushOnOpen = 1 << 25 };
enum ImGuiTabItemFlags_ { ImGuiTabItemFlags_None = 0, ImGuiTabItemFlags_UnsavedDocument = 1 << 0, ImGuiTabItemFlags_SetSelected = 1 << 1, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 1 << 2, ImGuiTabItemFlags_NoPushId = 1 << 3, ImGuiTabItemFlags_NoTooltip = 1 << 4, ImGuiTabItemFlags_NoReorder = 1 << 5, ImGuiTabItemFlags_Leading = 1 << 6, ImGuiTabItemFlags_Trailing = 1 << 7 };
enum ImGuiSelectableFlags_ { ImGuiSelectableFlags_None = 0, ImGuiSelectableFlags_DontClosePopups = 1 << 0, ImGuiSelectableFlags_SpanAllColumns = 1 << 1, ImGuiSelectableFlags_AllowDoubleClick = 1 << 2, ImGuiSelectableFlags_Disabled = 1 << 3, ImGuiSelectableFlags_AllowOverlap = 1 << 4 };
enum ImGuiComboFlags_ { ImGuiComboFlags_None = 0, ImGuiComboFlags_PopupAlignLeft = 1 << 0, ImGuiComboFlags_HeightSmall = 1 << 1, ImGuiComboFlags_HeightRegular = 1 << 2, ImGuiComboFlags_HeightLarge = 1 << 3, ImGuiComboFlags_HeightLargest = 1 << 4, ImGuiComboFlags_NoArrowButton = 1 << 5, ImGuiComboFlags_NoPreview = 1 << 6, ImGuiComboFlags_WidthFitPreview = 1 << 7 };
enum ImGuiInputTextFlags_ { ImGuiInputTextFlags_None = 0, ImGuiInputTextFlags_CharsDecimal = 1 << 0, ImGuiInputTextFlags_CharsHexadecimal = 1 << 1, ImGuiInputTextFlags_CharsUppercase = 1 << 2, ImGuiInputTextFlags_CharsNoBlank = 1 << 3, ImGuiInputTextFlags_AutoSelectAll = 1 << 4, ImGuiInputTextFlags_EnterReturnsTrue = 1 << 5, ImGuiInputTextFlags_CallbackCompletion = 1 << 6, ImGuiInputTextFlags_CallbackHistory = 1 << 7, ImGuiInputTextFlags_CallbackAlways = 1 << 8, ImGuiInputTextFlags_CallbackResize = 1 << 9, ImGuiInputTextFlags_CallbackEdit = 1 << 10, ImGuiInputTextFlags_ReadOnly = 1 << 11, ImGuiInputTextFlags_Password = 1 << 12, ImGuiInputTextFlags_NoUndoRedo = 1 << 13, ImGuiInputTextFlags_CharsScientific = 1 << 14 };
enum ImGuiHoveredFlags_ { ImGuiHoveredFlags_None = 0, ImGuiHoveredFlags_AllowWhenBlockedByPopup = 1 << 0, ImGuiHoveredFlags_AllowWhenBlockedByActiveItem = 1 << 1, ImGuiHoveredFlags_AllowWhenOverlappedByItem = 1 << 2, ImGuiHoveredFlags_AllowWhenDisabled = 1 << 3, ImGuiHoveredFlags_RectOnly = (1 << 0) | (1 << 1) | (1 << 2), ImGuiHoveredFlags_ForTooltip = 1 << 4, ImGuiHoveredFlags_Stationary = 1 << 5, ImGuiHoveredFlags_DelayNone = 1 << 6, ImGuiHoveredFlags_DelayShort = 1 << 7, ImGuiHoveredFlags_DelayNormal = 1 << 8, ImGuiHoveredFlags_NoNavOverride = 1 << 9 };
enum ImGuiPopupFlags_ { ImGuiPopupFlags_None = 0, ImGuiPopupFlags_MouseButtonLeft = 0, ImGuiPopupFlags_MouseButtonRight = 1, ImGuiPopupFlags_MouseButtonMiddle = 2, ImGuiPopupFlags_MouseButtonMask_ = 0x1F, ImGuiPopupFlags_MouseButtonDefault_ = 0, ImGuiPopupFlags_NoOpenOverExistingPopup = 1 << 5, ImGuiPopupFlags_NoOpenOverItems = 1 << 6, ImGuiPopupFlags_AnyPopupId = 1 << 7, ImGuiPopupFlags_AnyPopupLevel = 1 << 8, ImGuiPopupFlags_AnyPopup = 1 << 7 | 1 << 8 };
enum ImGuiChildFlags_ { ImGuiChildFlags_None = 0, ImGuiChildFlags_Border = 1 << 0, ImGuiChildFlags_Borders = 1 << 0, ImGuiChildFlags_AlwaysUseWindowPadding = 1 << 1, ImGuiChildFlags_ResizeX = 1 << 2, ImGuiChildFlags_ResizeY = 1 << 3, ImGuiChildFlags_AutoResizeX = 1 << 4, ImGuiChildFlags_AutoResizeY = 1 << 5, ImGuiChildFlags_AlwaysAutoResize = 1 << 6, ImGuiChildFlags_FrameStyle = 1 << 7 };

struct ImGuiIO {
  ImVec2 DisplaySize;
  float DeltaTime;
  float FontGlobalScale;
  ImVec2 DisplayFramebufferScale;
  int Fonts;
  void* UserData;
  float FontDefaultSize;
};
struct ImDrawList {
  static inline void AddLine(ImVec2, ImVec2, ImU32, float) {}
};

struct ImColor {
  ImVec4 Value;
  ImColor() : Value(0,0,0,0) {}
  ImColor(float r, float g, float b, float a = 1.0f) : Value(r, g, b, a) {}
  operator ImVec4() const { return Value; }
  static ImColor HSV(float h, float s, float v, float a = 1.0f) {
    float r, g, b;
    int i = int(h * 6); float f = h * 6 - i; float p = v * (1 - s); float q = v * (1 - f * s); float t = v * (1 - (1 - f) * s);
    switch (i % 6) { case 0: r = v; g = t; b = p; break; case 1: r = q; g = v; b = p; break; case 2: r = p; g = v; b = t; break; case 3: r = p; g = q; b = v; break; case 4: r = t; g = p; b = v; break; case 5: r = v; g = p; b = q; break; default: r = 0; g = 0; b = 0; }
    return ImColor(r, g, b, a);
  }
};

struct ImGuiStyle {
  ImVec2 WindowPadding;
  ImVec2 FramePadding;
  ImVec2 CellPadding;
  ImVec2 ItemSpacing;
  ImVec2 ItemInnerSpacing;
  ImVec2 TouchExtraPadding;
  ImVec2 WindowMinSize;
};

namespace ImGui {
  static inline ImGuiIO& GetIO() { static ImGuiIO io; return io; }
  static inline ImGuiStyle& GetStyle() { static ImGuiStyle style; return style; }

  static inline bool Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) { (void)name; (void)p_open; (void)flags; return true; }
  static inline void End() {}
  static inline bool BeginChild(const char* str_id, ImVec2 size = ImVec2(0,0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0) { (void)str_id; (void)size; (void)child_flags; (void)window_flags; return true; }
  static inline void EndChild() {}
  static inline bool BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0) { (void)label; (void)p_open; (void)flags; return true; }
  static inline void EndTabItem() {}
  static inline bool BeginPopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) { (void)name; (void)p_open; (void)flags; return false; }
  static inline void EndPopup() {}
  static inline void OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags = 0) { (void)str_id; (void)popup_flags; }
  static inline void CloseCurrentPopup() {}

  static inline bool Button(const char* label, ImVec2 size = ImVec2(0,0)) { (void)label; (void)size; return false; }
  static inline bool SmallButton(const char* label) { (void)label; return false; }
  static inline bool Checkbox(const char* label, bool* v) { (void)label; (void)v; return false; }
  static inline bool RadioButton(const char* label, bool active) { (void)label; (void)active; return false; }
  static inline bool RadioButton(const char* label, int* v, int v_button) { (void)label; (void)v; (void)v_button; return false; }
  static inline bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0) { (void)label; (void)v; (void)v_min; (void)v_max; (void)format; (void)flags; return false; }
  static inline bool SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0) { (void)label; (void)v; (void)v_min; (void)v_max; (void)format; (void)flags; return false; }
  static inline bool SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0) { (void)label; (void)v_rad; (void)v_degrees_min; (void)v_degrees_max; (void)format; (void)flags; return false; }
  static inline bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0) { (void)label; (void)v; (void)v_min; (void)v_max; (void)format; (void)flags; return false; }
  static inline bool InputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0) { (void)label; (void)v; (void)format; (void)flags; return false; }
  static inline bool InputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0) { (void)label; (void)v; (void)format; (void)flags; return false; }
  static inline bool Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1) { (void)label; (void)current_item; (void)items_separated_by_zeros; (void)popup_max_height_in_items; return false; }
  static inline bool Combo(const char* label, int* current_item, const char** items, int items_count, int popup_max_height_in_items = -1) { (void)label; (void)current_item; (void)items; (void)items_count; (void)popup_max_height_in_items; return false; }
  static inline bool Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, ImVec2 size = ImVec2(0,0)) { (void)label; (void)selected; (void)flags; (void)size; return false; }
  static inline bool CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0) { (void)label; (void)flags; return false; }
  static inline bool TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0) { (void)label; (void)flags; return false; }
  static inline void TreePop() {}
  static inline void SetNextItemOpen(bool is_open, ImGuiCond cond = 0) { (void)is_open; (void)cond; }
  static inline bool IsItemHovered(ImGuiHoveredFlags flags = 0) { (void)flags; return false; }
  static inline bool IsItemClicked(int mouse_button = 0) { (void)mouse_button; return false; }
  static inline bool IsItemToggledOpen() { return false; }
  static inline bool IsItemActivated() { return false; }

  static inline void Text(const char* fmt, ...) { va_list args; va_start(args, fmt); va_end(args); }
  static inline void TextColored(const ImVec4& col, const char* fmt, ...) { (void)col; (void)fmt; }
  static inline void TextDisabled(const char* fmt, ...) {}
  static inline void TextUnformatted(const char* text, const char* text_end = NULL) { (void)text; (void)text_end; }
  static inline void TextWrapped(const char* fmt, ...) {}
  static inline void LabelText(const char* label, const char* fmt, ...) { (void)label; }
  static inline void BulletText(const char* fmt, ...) {}
  static inline void SetTooltip(const char* fmt, ...) {}

  static inline void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f) { (void)offset_from_start_x; (void)spacing; }
  static inline void Separator() {}
  static inline void Spacing() {}
  static inline void NewLine() {}
  static inline void Indent(float indent_w = 0.0f) { (void)indent_w; }
  static inline void Unindent(float indent_w = 0.0f) { (void)indent_w; }
  static inline void PushItemWidth(float item_width) { (void)item_width; }
  static inline void PopItemWidth() {}
  static inline void PushStyleColor(ImGuiCol idx, ImU32 col) { (void)idx; (void)col; }
  static inline void PushStyleColor(ImGuiCol idx, const ImVec4& col) { (void)idx; (void)col; }
  static inline void PopStyleColor(int count = 1) { (void)count; }
  static inline void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0,0)) { (void)pos; (void)cond; (void)pivot; }
  static inline void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0) { (void)size; (void)cond; }
  static inline void SetScrollHereY(float center_y_ratio = 0.5f) { (void)center_y_ratio; }
  static inline void ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-1,0), const char* overlay = NULL) { (void)fraction; (void)size_arg; (void)overlay; }
  static inline void InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0) { (void)label; (void)v; (void)step; (void)step_fast; (void)flags; }
  static inline bool InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, void* callback = NULL, void* user_data = NULL) { (void)label; (void)hint; (void)buf; (void)buf_size; (void)flags; (void)callback; (void)user_data; return false; }
  static inline bool InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0) { (void)label; (void)v; (void)step; (void)step_fast; (void)format; (void)flags; return false; }
  static inline bool ColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags = 0) { (void)label; (void)col; (void)flags; return false; }
  static inline void ColorConvertU32ToFloat4(ImVec4* out, ImU32 in) { (void)out; (void)in; }
  static inline ImVec4 ColorConvertU32ToFloat4(ImU32 in) { (void)in; return ImVec4(0,0,0,0); }

  static inline float GetFontSize() { return 13.0f; }
  static inline int GetFrameHeight() { return 20; }
  static inline ImVec2 GetItemRectMin() { return ImVec2(0,0); }
  static inline ImVec2 GetItemRectMax() { return ImVec2(0,0); }
  static inline ImVec2 GetWindowPos() { return ImVec2(0,0); }
  static inline ImVec2 GetWindowSize() { return ImVec2(1920, 1080); }
  static inline float GetWindowWidth() { return 1920.0f; }
  static inline float GetWindowHeight() { return 1080.0f; }
  static inline ImDrawList* GetWindowDrawList() { static ImDrawList dl; return &dl; }
  static inline ImGuiID GetID(const char* str_id) { (void)str_id; return 0; }

  static inline void BeginTooltip() {}
  static inline void EndTooltip() {}
  static inline void PushID(const char* str_id) { (void)str_id; }
  static inline void PushID(int int_id) { (void)int_id; }
  static inline void PopID() {}
  static inline void Dummy(const ImVec2& size) { (void)size; }
  static inline void AlignTextToFramePadding() {}
  static inline float CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f) { (void)text; (void)text_end; (void)hide_text_after_double_hash; (void)wrap_width; return 0.0f; }
  static inline bool IsWindowAppearing() { return false; }
  static inline bool IsWindowCollapsed() { return false; }
  static inline bool IsWindowFocused(int flags = 0) { (void)flags; return false; }
  static inline bool IsAnyMouseDown() { return false; }
  static inline bool IsMouseDown(int button) { (void)button; return false; }
  static inline bool IsMouseClicked(int button, bool repeat = false) { (void)button; (void)repeat; return false; }
  static inline bool IsMouseReleased(int button) { (void)button; return false; }
  static inline bool IsMouseDragging(int button = 0, float lock_threshold = -1.0f) { (void)button; (void)lock_threshold; return false; }
  static inline ImVec2 GetMousePos() { return ImVec2(0,0); }
  static inline ImVec2 GetMouseDragDelta(int button = 0, float lock_threshold = -1.0f) { (void)button; (void)lock_threshold; return ImVec2(0,0); }
  static inline void ResetMouseDragDelta(int button = 0) { (void)button; }
  static inline void SetCursorPosX(float local_x) { (void)local_x; }
  static inline void SetCursorPosY(float local_y) { (void)local_y; }
  static inline float GetCursorPosX() { return 0; }
  static inline float GetCursorPosY() { return 0; }
  static inline ImVec2 GetCursorScreenPos() { return ImVec2(0,0); }
  static inline float GetContentRegionAvailX() { return 1920; }
  static inline float GetContentRegionAvailY() { return 1080; }
  static inline ImVec2 GetContentRegionAvail() { return ImVec2(1920, 1080); }
  static inline void SetCursorScreenPos(const ImVec2& pos) { (void)pos; }
  static inline void SetItemDefaultFocus() {}
  static inline void SetKeyboardFocusHere(int offset = 0) { (void)offset; }
  static inline void PushTextWrapPos(float wrap_pos_x = 0.0f) { (void)wrap_pos_x; }
  static inline void PopTextWrapPos() {}
  static inline void PushFont(void* font) { (void)font; }
  static inline void PopFont() {}
  static inline void PushAllowKeyboardFocus(bool v) { (void)v; }
  static inline void PopAllowKeyboardFocus() {}
  static inline void PushButtonRepeat(bool repeat) { (void)repeat; }
  static inline void PopButtonRepeat() {}
  static inline void SetColumnOffset(int column_index, float offset_x) { (void)column_index; (void)offset_x; }
  static inline float GetColumnOffset(int column_index) { (void)column_index; return 0; }
  static inline void SetColumnWidth(int column_index, float width) { (void)column_index; (void)width; }
  static inline float GetColumnWidth(int column_index) { (void)column_index; return 0; }
  static inline int GetColumnsCount() { return 1; }
  static inline void Columns(int count = 1, const char* id = NULL, bool border = true) { (void)count; (void)id; (void)border; }
  static inline void NextColumn() {}
  static inline bool DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0) { (void)label; (void)v; (void)v_speed; (void)v_min; (void)v_max; (void)format; (void)flags; return false; }
}
