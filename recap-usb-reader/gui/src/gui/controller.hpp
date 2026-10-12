#pragma once

#include "device.hpp"
#include "directory.hpp"
#include "writer-observer.hpp"

#include <memory>

namespace recap::app::gui::controller {

class Controller {
public:
    Controller(
        recap::app::device::Device& device, recap::app::writer_observer::WriterObserver& writer
    );
    ~Controller();
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
    Controller(Controller&&) = delete;
    Controller& operator=(Controller&&) = delete;

    void update_and_draw();

    class State;

private:
    template <class S, class... Args> void set_state(Args&&...args);

    recap::app::device::Device& m_device;
    recap::app::writer_observer::WriterObserver& m_writer;
    std::unique_ptr<State> m_state;
    recap::app::gui::DirectorySelector m_dir_selector;

    recap::app::device::Device::ConnectedObservable::Observer m_connect_sub;
    recap::app::device::Device::DisconnectedObservable::Observer m_disconnect_sub;
};

}; // namespace recap::app::gui::controller
