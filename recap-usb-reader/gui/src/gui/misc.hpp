#pragma once

#include <imgui.h>

namespace recap::app::gui {

void HelpMarker(const char *desc);

void PushButtonColor(const ImColor& normal, const ImColor& hovered, const ImColor& active);

// Set color of all three button states (normal, hovered, active)
void PushButtonColor(const ImColor& color);

inline ImU32 ButtonColorNormal(float hue)
{
    return ImColor::HSV(hue, 0.6F, 0.6F);
}

inline ImU32 ButtonColorHovered(float hue)
{
    return ImColor::HSV(hue, 0.7F, 0.7F);
}

inline ImU32 ButtonColorActive(float hue)
{
    return ImColor::HSV(hue, 0.8F, 0.8F);
}

// Set color theme of button with different shades for the three states (normal, hovered, active)
void PushButtonColorHSV(float hue);

void PopButtonColor();

inline const char *ellipsis_animated()
{
    static constexpr const char *dots[] = {"", ".", "..", "..."};
    return dots[static_cast<int>(ImGui::GetTime() / .25f) & 3];
}

}; // namespace recap::app::gui
