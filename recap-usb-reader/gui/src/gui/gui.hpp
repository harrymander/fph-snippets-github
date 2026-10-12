#pragma once

#include "controller.hpp"
#include "device.hpp"
#include "hidable.hpp"
#include "plots.hpp"
#include "writer-observer.hpp"

#include <config/app.h>

#include <imgui.h>

#include <stdexcept>

namespace recap::app::gui {

struct GuiException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct ImGuiDemoWindow : public recap::app::gui::hidable::Hidable {
    void draw_contents() override;
};

struct ImPlotDemoWindow : public recap::app::gui::hidable::Hidable {
    void draw_contents() override;
};

struct MetricsWindow : public recap::app::gui::hidable::Hidable {
    void draw_contents() override;
};

struct AboutWindow : public recap::app::gui::hidable::Hidable {
    void draw_contents() override;
};

struct LicenseWindow : public recap::app::gui::hidable::Hidable {
    void draw_contents() override;

private:
    std::size_t m_num_expanded = 0;
};

class Gui {
public:
    Gui(float scaling_factor, recap::app::device::Device& device);
    ~Gui();
    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;
    Gui(Gui&&) = delete;
    Gui& operator=(Gui&&) = delete;

    bool draw();
    void stop();

private:
    void handle_stopping();
    void draw_stopping_modal();
    void draw_controller_view();
    void draw_sensor_view();
    void draw_menu_bar();
    void draw_about_window();

    recap::app::device::Device& m_device;
    recap::app::writer_observer::WriterObserver m_writer;
    recap::app::gui::controller::Controller m_controller;
    recap::app::gui::plots::PlotsView m_plots;

    enum class State {
        Running,
        Stopping,
        Stopped,
    };

    State m_state = State::Running;

    int m_style_id = 0;
    MetricsWindow m_metrics_window;
    AboutWindow m_about_window;
    LicenseWindow m_license_window;

#ifdef RECAP_USB_READER_GUI_INCLUDE_DEMOS
    ImGuiDemoWindow m_imgui_demo_window;
    ImPlotDemoWindow m_implot_demo_window;
#endif
};

}; // namespace recap::app::gui
