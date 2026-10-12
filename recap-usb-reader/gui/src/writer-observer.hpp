#pragma once

#include "device.hpp"
#include "observable.hpp"

#include <recap/usb-reader/record.hpp>

#include <filesystem>
#include <memory>
#include <optional>

namespace recap::app::writer_observer {

class WriterObserver {
public:
    explicit WriterObserver(recap::app::device::Device& device);
    ~WriterObserver();

    WriterObserver(const WriterObserver&) = delete;
    WriterObserver& operator=(const WriterObserver&) = delete;
    WriterObserver(WriterObserver&&) = delete;
    WriterObserver& operator=(WriterObserver&&) = delete;

    const std::optional<std::filesystem::path>& next_output_path() const;
    void set_next_output_path(const std::filesystem::path& path);
    void clear_next_output_path();

    using ErrorObservable = recap::app::observable::Observable<const std::string&>;
    ErrorObservable::Observer subscribe_error(const ErrorObservable::Function& func);

private:
    class Observer;

    ErrorObservable m_error_obs;
    std::unique_ptr<Observer> m_observer;
    std::optional<std::filesystem::path> m_next_output_path = std::nullopt;

    recap::app::device::Device& m_device;
    recap::app::device::Device::NewRecordingObservable::Observer m_new_recording_sub;
    recap::app::device::Device::EndRecordingObservable::Observer m_end_recording_sub;
    recap::app::device::Device::DisconnectedObservable::Observer m_disconnected_sub;

    void reset();
    void error(const std::string& err);
    void on_new_recording(
        const recap::record::RecordVersion& record_version, const recap::record::Header& header
    );
};

}; // namespace recap::app::writer_observer
