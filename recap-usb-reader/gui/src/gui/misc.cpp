#include "misc.hpp"

#include <imgui.h>

namespace recap::app::gui {

// From imgui_demo.cpp
void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void PushButtonColor(const ImColor& normal, const ImColor& hovered, const ImColor& active)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(normal));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active));
}

void PushButtonColor(const ImColor& color)
{
    PushButtonColor(color, color, color);
}

void PushButtonColorHSV(float hue)
{
    PushButtonColor(ButtonColorNormal(hue), ButtonColorHovered(hue), ButtonColorActive(hue));
}

void PopButtonColor()
{
    ImGui::PopStyleColor(3);
}

}; // namespace recap::app::gui
