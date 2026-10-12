#pragma once

#include "observable.hpp"

#include <recap/usb-reader/reader.hpp>
#include <recap/usb-reader/record.hpp>

#include <memory>
#include <string>

namespace recap::app::device {

class Device {
public:
    Device();
    ~Device();

    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

    void update();

    enum class State {
        NotConnected,
        Error,
        Detecting,
        Idle,
        Recording,
        Stopping,
    };

    State state() const;

    void detect_device();
    void start_recording(recap::record::CompleteSample::raw_millis_t duration = 0);
    void stop_recording();
    void set_event_override(bool);

    // Takes a void(const std::string& error) callback
    using ErrorObservable = recap::app::observable::Observable<const std::string&>;
    ErrorObservable::Observer subscribe_error(const ErrorObservable::Function& func);

    struct ConnectedDeviceInfo {
        const std::string port_name;
        const std::string version_str;
        const std::string build_digest;
    };

    // Takes a void(const ConnectedDeviceInfo& info) callback
    using ConnectedObservable = recap::app::observable::Observable<const ConnectedDeviceInfo&>;
    ConnectedObservable::Observer subscribe_connected(const ConnectedObservable::Function& func);

    // Takes a void(const recap::record::RecordVersion&, const recap::record::Header&) callback
    using NewRecordingObservable = recap::app::observable::
        Observable<const recap::record::RecordVersion&, const recap::record::Header&>;
    NewRecordingObservable::Observer
    subscribe_new_recording(const NewRecordingObservable::Function& func);

    // Takes a void(const recap::record::Sample *) callback
    using NewSampleObservable = recap::app::observable::Observable<const recap::record::Sample *>;
    NewSampleObservable::Observer subscribe_new_sample(const NewSampleObservable::Function& func);

    using EndRecordingObservable = recap::app::observable::Observable<>;
    EndRecordingObservable::Observer
    subscribe_end_recording(const EndRecordingObservable::Function& func);

    // Takes a void(const std::string *error) callback where error is an optional error message
    using DisconnectedObservable = recap::app::observable::Observable<const std::string *>;
    DisconnectedObservable::Observer
    subscribe_disconnected(const DisconnectedObservable::Function& func);

    class StateManager;

private:
    template <class S, class... Args> void set_state(Args&&...args);

    std::unique_ptr<StateManager> m_state;
    ConnectedObservable m_connected_observable;
    NewRecordingObservable m_new_recording_observable;
    NewSampleObservable m_new_sample_observable;
    EndRecordingObservable m_end_recording_observable;
    DisconnectedObservable m_disconnected_observable;
    bool m_event_state = false;
};

}; // namespace recap::app::device
