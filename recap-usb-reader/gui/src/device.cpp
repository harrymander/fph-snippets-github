// Avoids warnings about gmtime on MSVC
#define _CRT_SECURE_NO_WARNINGS

#include "device.hpp"

#include <recap/usb-reader/reader.hpp>
#include <recap/usb-reader/record.hpp>
#include <utils/battery.hpp>
#include <utils/datetimestr.hpp>

#include <chrono>
#include <ctime>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

using recap::reader::Reader;
using recap::reader::RecordStream;
using recap::record::CompleteSample;
using recap::record::Header;
using recap::record::InvalidRecordHeaderError;
using recap::record::RecordVersion;
using recap::record::Sample;
using namespace std::string_literals;

namespace recap::app::device {

class Device::StateManager {
public:
    explicit StateManager(Device& device) : m_device(device) {}

    virtual ~StateManager() = default;
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;

    virtual void update() {}

    virtual Device::State state() const = 0;

    virtual void detect_device() {}

    virtual void start_recording([[maybe_unused]] CompleteSample::raw_millis_t duration) {}

    virtual void stop_recording() {}

    virtual void set_event_override([[maybe_unused]] bool override) {}

    template <class S, class... Args> void set_state(Args&&...args)
    {
        m_device.set_state<S>(std::forward<Args>(args)...);
    }

    void notify_new_sample(const std::unique_ptr<Sample>& sample)
    {
        m_device.m_new_sample_observable.notify(sample.get());
    }

    void notify_connected(const Device::ConnectedDeviceInfo& info)
    {
        m_device.m_connected_observable.notify(info);
    }

    void notify_new_recording(const RecordVersion& record_version, const Header& header)
    {
        m_device.m_new_recording_observable.notify(record_version, header);
    }

    void notify_end_recording() { m_device.m_end_recording_observable.notify(); }

    void notify_not_connected(const std::string *error)
    {
        m_device.m_disconnected_observable.notify(error);
    }

protected:
    [[nodiscard]] bool event_state() const { return m_device.m_event_state; }

private:
    Device& m_device;
};

class NotConnectedState;

Device::Device()
{
    set_state<NotConnectedState>();
}

Device::~Device() = default;

template <class S, class... Args> void Device::set_state(Args&&...args)
{
    m_state = std::make_unique<S>(*this, std::forward<Args>(args)...);
}

void Device::update()
{
    m_state->update();
}

Device::State Device::state() const
{
    return m_state->state();
}

void Device::detect_device()
{
    m_state->detect_device();
}

void Device::start_recording(record::CompleteSample::raw_millis_t duration)
{
    m_state->start_recording(duration);
}

void Device::stop_recording()
{
    m_state->stop_recording();
}

void Device::set_event_override(bool override)
{
    m_state->set_event_override(override);
}

Device::ConnectedObservable::Observer
Device::subscribe_connected(const ConnectedObservable::Function& func)
{
    return m_connected_observable.subscribe(func);
}

Device::NewRecordingObservable::Observer
Device::subscribe_new_recording(const NewRecordingObservable::Function& func)
{
    return m_new_recording_observable.subscribe(func);
}

Device::NewSampleObservable::Observer
Device::subscribe_new_sample(const NewSampleObservable::Function& func)
{
    return m_new_sample_observable.subscribe(func);
}

Device::EndRecordingObservable::Observer
Device::subscribe_end_recording(const EndRecordingObservable::Function& func)
{
    return m_end_recording_observable.subscribe(func);
}

Device::DisconnectedObservable::Observer
Device::subscribe_disconnected(const DisconnectedObservable::Function& func)
{
    return m_disconnected_observable.subscribe(func);
}

namespace {

std::string battery_status_str(const std::optional<utils::BatteryInfo>& info)
{
    if (!info.has_value()) {
        return "Error getting battery status";
    }

    using enum utils::BatteryInfo::Status;
    std::string status;
    switch (info->status) {
    case NotCharging:
        status = "Not charging";
        break;
    case Charging:
        status = "Charging";
        break;
    case NoBattery:
        return "No battery";
    }

    std::string percent = info->percent ? std::to_string(*info->percent) : "Unknown";
    return status + ", " + percent + "%";
}

}; // namespace

class IdleState;

class RecordingState : public Device::StateManager {
public:
    RecordingState(
        Device& device,
        std::unique_ptr<Reader>&& reader,
        RecordStream&& stream,
        CompleteSample::raw_millis_t duration
    ) :
        Device::StateManager(device),
        m_reader(std::move(reader)),
        m_stream(std::move(stream)),
        m_header(m_stream.header()),
        m_duration(duration)
    {
        m_header.set_extra_field("Host", BUILD_INFO);
        m_header.set_extra_field("Datetime", utils::utc_datetime_str());
        m_header.set_extra_field("Host-Battery", battery_status_str(utils::get_battery_info()));
        notify_new_recording(m_stream.version(), m_header);
    }

    Device::State state() const override { return m_state; }

    void stop_recording() override
    {
        if (m_state == Device::State::Recording) {
            m_reader->stop_recording();
            m_state = Device::State::Stopping;
        }
    }

    void set_event_override(bool override) override { m_stream.set_event_override(override); }

    void update() override
    {
        int timeout = 1000;
        while (1) {
            try {
                std::unique_ptr<Sample> sample;
                switch (m_stream.read_sample(sample, timeout)) {
                    using enum RecordStream::ReadSampleResult;

                case Ok:
                    if (event_state()) {
                    }
                    notify_new_sample(sample);
                    if (m_duration && sample->millis() >= m_duration
                        && m_state == Device::State::Recording)
                    {
                        stop_recording();
                    }
                    break;
                case Sentinel:
                    m_state = Device::State::Idle;
                    notify_end_recording();
                    set_state<IdleState>(std::move(m_reader));
                    return;
                case Timeout:
                    if (timeout) {
                        m_state = Device::State::Error;
                        set_state<NotConnectedState>("Timed-out reading sample");
                    }
                    return;
                }
            } catch (const std::runtime_error& error) {
                m_state = Device::State::Error;
                set_state<NotConnectedState>("Error reading from device: "s + error.what());
                return;
            }
            timeout = 0;
        }
    }

private:
    Device::State m_state = Device::State::Recording;

    std::unique_ptr<Reader> m_reader;
    RecordStream m_stream;
    Header m_header;
    CompleteSample::raw_millis_t m_duration;
};

class IdleState : public Device::StateManager {
public:
    IdleState(Device& device, std::unique_ptr<Reader>&& reader) :
        Device::StateManager(device), m_reader(std::move(reader))
    {}

    Device::State state() const override { return Device::State::Idle; }

    void start_recording(CompleteSample::raw_millis_t duration) override
    {
        try {
            auto stream = m_reader->start_recording();
            if (!stream) {
                set_state<NotConnectedState>("Timed-out starting recording");
            } else {
                set_state<RecordingState>(std::move(m_reader), std::move(*stream), duration);
            }
        } catch (const InvalidRecordHeaderError& error) {
            set_state<NotConnectedState>("Invalid record header: "s + error.what());
            return;
        } catch (const std::runtime_error& error) {
            set_state<NotConnectedState>("Error starting recording: "s + error.what());
        }
    }

private:
    std::unique_ptr<Reader> m_reader;
};

class DetectingState : public Device::StateManager {
public:
    explicit DetectingState(Device& device) :
        Device::StateManager(device),
        m_fut(std::async(std::launch::async, []() -> std::optional<fut_ret> {
            std::unique_ptr<Reader> reader;
            try {
                reader = std::make_unique<Reader>();
            } catch (const std::runtime_error&) {
                return std::nullopt;
            }

            std::optional<record::FirmwareVersion> version;
            try {
                version = reader->query_version();
            } catch (const std::invalid_argument& error) {
                throw std::runtime_error("invalid version: "s + error.what());
            }
            if (!version) {
                throw std::runtime_error("error getting version string");
            }
            const auto build_digest = reader->query_build_id();
            if (!build_digest) {
                throw std::runtime_error("error getting build ID");
            }

            return std::make_optional<fut_ret>({std::move(reader), version->str(), *build_digest});
        }))
    {}

    void update() override
    {
        if (is_ready()) {
            get_device();
        }
    }

    constexpr Device::State state() const override { return Device::State::Detecting; }

private:
    void get_device()
    {
        try {
            auto ret = m_fut.get();
            if (ret.has_value()) {
                const Device::ConnectedDeviceInfo info{
                    .port_name = ret->reader->get_port(),
                    .version_str = ret->version_str,
                    .build_digest = ret->build_digest,
                };
                notify_connected(info);
                set_state<IdleState>(std::move(ret->reader));
            } else {
                set_state<NotConnectedState>();
            }
        } catch (const std::runtime_error& error) {
            set_state<NotConnectedState>(error.what());
        }
    }

    bool is_ready() const
    {
        return m_fut.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

    struct fut_ret {
        std::unique_ptr<Reader> reader;
        std::string version_str;
        std::string build_digest;
    };

    std::future<std::optional<fut_ret>> m_fut;
};

class NotConnectedState : public Device::StateManager {
public:
    explicit NotConnectedState(Device& device, std::string_view error = "") :
        Device::StateManager(device), m_error(error)
    {
        notify_not_connected(m_error.empty() ? nullptr : &m_error);
    }

    void detect_device() override { set_state<DetectingState>(); }

    Device::State state() const override
    {
        return m_error.empty() ? Device::State::NotConnected : Device::State::Error;
    }

private:
    std::string m_error;
};

}; // namespace recap::app::device
