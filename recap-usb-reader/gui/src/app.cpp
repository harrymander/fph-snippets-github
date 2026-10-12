#include "app.hpp"

#include "device.hpp"
#include "gui/gui.hpp"

#include <imgui.h>
#include <implot.h>

#include <iostream>
#include <memory>

namespace recap::app {

using recap::app::device::Device;
using recap::app::gui::Gui;

class App {
public:
    explicit App(float scaling_factor) : gui(scaling_factor, device) { device.detect_device(); }

    bool update()
    {
        device.update();
        return gui.draw();
    }

    void close() { gui.stop(); }

private:
    Device device;
    Gui gui;
};

static std::unique_ptr<App> g_app_ptr = nullptr;

bool ShowApp()
{
    return g_app_ptr->update();
}

void CloseApp()
{
    g_app_ptr->close();
}

int SetupApp(float scaling_factor)
{
    try {
        g_app_ptr = std::make_unique<App>(scaling_factor);
    } catch (const gui::GuiException& exc) {
        std::cerr << "Error initialising GUI: " << exc.what() << '\n';
        return 1;
    }
    return 0;
}

void TearDownApp()
{
    g_app_ptr.reset();
}

}; // namespace recap::app
