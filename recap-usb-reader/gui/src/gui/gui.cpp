#include "gui.hpp"

#include "device.hpp"
#include "font.hpp"
#include "gui/misc.hpp"
#include "license-info.hpp"

#include <config/app.h>

#include <imgui.h>
#include <implot.h>

using namespace recap::app;
using namespace recap::app::device;
using namespace recap::app::gui;

Gui::Gui(float scaling_factor, Device& dev) :
    m_device(dev), m_writer(m_device), m_controller(m_device, m_writer), m_plots(m_device)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    if (scaling_factor > 0) {
        ImGui::GetStyle().ScaleAllSizes(scaling_factor);
    }

    if (font::init_fonts(scaling_factor) < 0) {
        throw GuiException("Error setting up fonts\n");
    }
}

Gui::~Gui()
{
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

bool Gui::draw()
{
    static constexpr ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    const auto io = ImGui::GetIO();

    // Use a window as a pane that fills the vertical space and is only resizable width-wise
    ImGui::SetWindowSize(ImVec2(-1., io.DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(0, io.DisplaySize.y), ImVec2(io.DisplaySize.x, io.DisplaySize.y)
    );
    ImGui::Begin("RECAP", nullptr, window_flags | ImGuiWindowFlags_MenuBar);
    draw_controller_view();
    const auto ctrl_window_size = ImGui::GetWindowSize();
    ImGui::End();

    // Draw another 'window' that is constrained to the remaining size of the parent window
    ImGui::SetNextWindowPos(ImVec2(ctrl_window_size.x, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - ctrl_window_size.x, io.DisplaySize.y));
    ImGui::Begin(
        "Sensor data",
        nullptr,
        window_flags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
    );
    draw_sensor_view();
    ImGui::End();

    m_metrics_window.draw();
    m_about_window.draw();
    m_license_window.draw();
#ifdef RECAP_USB_READER_GUI_INCLUDE_DEMOS
    m_implot_demo_window.draw();
    m_imgui_demo_window.draw();
#endif

    if (m_state == State::Stopping) {
        handle_stopping();
    }
    return m_state != State::Stopped;
}

void Gui::stop()
{
    m_state = State::Stopping;
}

void Gui::handle_stopping()
{
    using enum Device::State;
    const auto device_state = m_device.state();
    if (device_state == Recording || device_state == Stopping) {
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(300, -1), ImGuiCond_Always);
        ImGui::OpenPopup("Close?");
        if (ImGui::BeginPopupModal("Close?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            draw_stopping_modal();
            ImGui::EndPopup();
        }
    } else {
        m_state = State::Stopped;
    }
}

void Gui::draw_stopping_modal()
{
    using enum Device::State;
    const auto device_state = m_device.state();
    if (device_state == Recording) {
        ImGui::TextWrapped("The device is still recording!");
        if (ImGui::Button("Cancel", ImVec2(140, 0))) {
            m_state = State::Running;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Stop and close", ImVec2(140, 0))) {
            m_device.stop_recording();
        }
    } else if (device_state == Stopping) {
        ImGui::Text("Stopping%s", ellipsis_animated());
    }
}

void Gui::draw_controller_view()
{
    if (ImGui::BeginMenuBar()) {
        draw_menu_bar();
        ImGui::EndMenuBar();
    }

    m_controller.update_and_draw();
}

void Gui::draw_sensor_view()
{
    m_plots.draw();
}

void Gui::draw_menu_bar()
{
    if (ImGui::BeginMenu("View")) {
        if (ImGui::Combo("Colour scheme", &m_style_id, "Dark\0Light\0Classic\0")) {
            switch (m_style_id) {
            case 0:
                ImGui::StyleColorsDark();
                break;
            case 1:
                ImGui::StyleColorsLight();
                break;
            case 2:
                ImGui::StyleColorsClassic();
                break;
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem("Metrics/debugger", nullptr, m_metrics_window.status_ptr());
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("About", nullptr, m_about_window.status_ptr());
        ImGui::MenuItem("Licenses", nullptr, m_license_window.status_ptr());
#ifdef RECAP_USB_READER_GUI_INCLUDE_DEMOS
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, m_imgui_demo_window.status_ptr());
        ImGui::MenuItem("ImPlot Demo", nullptr, m_implot_demo_window.status_ptr());
#endif // RECAP_USB_READER_GUI_INCLUDE_DEMOS
        ImGui::EndMenu();
    }
}

void ImGuiDemoWindow::draw_contents()
{
    ImGui::ShowDemoWindow(status_ptr());
}

void ImPlotDemoWindow::draw_contents()
{
    ImPlot::ShowDemoWindow(status_ptr());
}

void MetricsWindow::draw_contents()
{
    ImGui::ShowMetricsWindow(status_ptr());
}

void AboutWindow::draw_contents()
{
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(300, -1), ImGuiCond_FirstUseEver);

    ImGui::Begin("About", status_ptr(), ImGuiWindowFlags_NoCollapse);
    ImGui::TextWrapped("%s", BUILD_INFO);
    ImGui::End();
}

void LicenseWindow::draw_contents()
{
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Licenses", status_ptr(), ImGuiWindowFlags_NoCollapse);

    ImGui::PushTextWrapPos(0);
    ImGui::TextUnformatted(
        "The following are the licenses of the open-source projects used to build this application"
    );

    std::optional<bool> expand = std::nullopt;
    ImGui::BeginDisabled(m_num_expanded == LicenseInfos.size());
    if (ImGui::Button("Expand all")) {
        expand = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(m_num_expanded == 0);
    if (ImGui::Button("Collapse all")) {
        expand = false;
    }
    ImGui::EndDisabled();

    m_num_expanded = 0;
    for (std::size_t i = 0; i < LicenseInfos.size(); i++) {
        ImGui::PushID(i);
        const auto& license_info = LicenseInfos[i];
        if (expand.has_value()) {
            ImGui::SetNextItemOpen(*expand);
        }
        if (ImGui::CollapsingHeader(license_info.package_name)) {
            m_num_expanded += 1;
            ImGui::TextUnformatted(license_info.license_text);
        }
        ImGui::PopID();
    }

    ImGui::PopTextWrapPos();
    ImGui::End();
}
