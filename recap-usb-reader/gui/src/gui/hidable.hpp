#pragma once

#include <imgui.h>

namespace recap::app::gui::hidable {
class Hidable {
public:
    explicit Hidable(bool enabled = false) : m_enabled(enabled) {}

    Hidable(const Hidable&) = default;
    Hidable(Hidable&&) = default;
    Hidable& operator=(const Hidable&) = default;
    Hidable& operator=(Hidable&&) = default;

    virtual ~Hidable() = default;

    virtual void draw_contents() = 0;

    void draw()
    {
        if (m_enabled) {
            draw_contents();
        }
    }

    bool status() const { return m_enabled; }

    void set_status(bool status) { m_enabled = status; };

    void show() { set_status(true); }

    void hide() { set_status(false); }

    void toggle() { set_status(!status()); }

    bool *status_ptr() { return &m_enabled; }

private:
    bool m_enabled;
};
}; // namespace recap::app::gui::hidable
