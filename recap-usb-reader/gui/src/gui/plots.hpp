#pragma once

#include "device.hpp"

#include <memory>

namespace recap::app::gui::plots {

class PlotsView {
public:
    explicit PlotsView(recap::app::device::Device& device);

    ~PlotsView();
    PlotsView(const PlotsView&) = delete;
    PlotsView(PlotsView&&) = default;
    PlotsView& operator=(const PlotsView&) = delete;
    PlotsView& operator=(PlotsView&&) = default;

    void draw();

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;
};

}; // namespace recap::app::gui::plots
