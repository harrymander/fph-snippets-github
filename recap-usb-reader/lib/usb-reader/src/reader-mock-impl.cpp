#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "reader.hpp"
#include "record.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
// clang-format off
#define timeBeginPeriod(x) do {} while (0)
#define timeEndPeriod(x) do {} while (0)
// clang-format on
#endif

namespace recap::reader {

using namespace recap::record;

static const std::string MockBuildDigest = "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef";
static const FirmwareVersion MockVersionInfo(4, 0, 0, "mock");
constexpr CompleteSample::raw_millis_t StopDelayMillis = 500;
static constexpr unsigned int SampleRate_kHz = 2;
static constexpr std::chrono::microseconds SamplePeriod(1'000 / SampleRate_kHz);
static const Header MockRecordHeader(
    MockVersionInfo,
    MockBuildDigest,
    SampleRate_kHz,
    5000,
    "Mocked reader",
    "Mocked SFM3003-300-CE flow meter",
    -12288,
    120,
    true,
    "Mocked H2S sensor"
);

struct Reader::Impl {
    enum class State {
        Idle,
        Recording,
        Stopping,
    };

    State m_state = State::Idle;

    void start()
    {
        m_state = State::Recording;
        m_sample_millis = 0;
        m_audio_ticks = SampleRate_kHz;
        m_next_sample_time = Clock::now() + SamplePeriod;
    }

    void stop()
    {
        m_state = State::Stopping;
        m_stopping_millis = 0;
    }

    std::size_t read(std::vector<uint8_t>& buffer, std::size_t size, Reader::timeout_t timeout)
    {
        const auto timeout_stop_time = Clock::now() + std::chrono::milliseconds(timeout);
        if ((m_state == State::Idle) || (timeout_stop_time < m_next_sample_time)) {
            std::this_thread::sleep_until(timeout_stop_time);
            return 0;
        }

        std::this_thread::sleep_until(m_next_sample_time);
        return next_sample(buffer, size);
    }

private:
    using Clock = std::chrono::steady_clock;

    uint32_t m_sample_millis = 0;
    uint32_t m_stopping_millis = 0;
    uint32_t m_audio_ticks = 0;
    Clock::time_point m_next_sample_time;
    std::mt19937 m_rng;

    double random_audio_volts()
    {
        static std::normal_distribution<> acoustic_noise(0, 2);
        return 2.5 + acoustic_noise(m_rng);
    }

    std::size_t push_random_audio_sample(std::vector<uint8_t>& dest, std::size_t size)
    {
        AudioSampleBuilder builder(MockRecordHeader);
        builder.set_mask_volts(random_audio_volts()).set_ear_volts(random_audio_volts());

        const auto to_write = std::min(size, builder.buffer().size());
        dest.insert(dest.end(), builder.buffer().begin(), builder.buffer().begin() + to_write);
        return to_write;
    }

    std::size_t
    push_random_complete_sample(std::vector<uint8_t>& dest, std::size_t size, uint32_t millis)
    {
        static std::normal_distribution<> air_noise(0, 15);
        auto& h2s_noise = air_noise;

        CompleteSampleBuilder builder(MockRecordHeader);

        builder.set_millis(millis)
            .set_pressure_pascals(50. * std::sin((double) millis * M_PI / 125.) + air_noise(m_rng))
            .set_flow_lpm(80. * std::sin((double) millis * M_PI / 250.) + air_noise(m_rng))
            .set_mask_volts(random_audio_volts())
            .set_ear_volts(random_audio_volts())
            .set_event((millis % 1000) < 500);

        if ((millis % 1000) == 0) {
            builder.set_h2s_conc_ppb(static_cast<int32_t>(std::round(100 * h2s_noise(m_rng))))
                .set_h2s_temp_celsius(static_cast<int8_t>(std::round(h2s_noise(m_rng))))
                .set_h2s_humidity_percent(static_cast<uint8_t>(std::round(50 + h2s_noise(m_rng))));
        } else {
            builder.clear_h2s();
        }

        const auto to_write = std::min(size, builder.buffer().size());
        dest.insert(dest.end(), builder.buffer().begin(), builder.buffer().begin() + to_write);
        return to_write;
    }

    std::size_t next_sample(std::vector<uint8_t>& buffer, std::size_t size)
    {
        std::size_t r;
        if (m_audio_ticks < SampleRate_kHz - 1) {
            m_audio_ticks += 1;
            r = push_random_audio_sample(buffer, size);
        } else {
            m_audio_ticks = 0;
            if (m_state == State::Stopping) {
                m_stopping_millis += 1;
                if (m_stopping_millis >= StopDelayMillis) {
                    m_state = State::Idle;
                    return push_random_complete_sample(buffer, size, 0);
                }
            }
            m_sample_millis += 1;
            r = push_random_complete_sample(buffer, size, m_sample_millis);
        }

        m_next_sample_time += SamplePeriod;
        return r;
    }
};

Reader::Reader(std::optional<std::string> /* unused */) : m_pimpl(std::make_unique<Impl>()) {}

Reader::~Reader() = default;

std::optional<RecordStream> Reader::start_recording()
{
    m_pimpl->start();
    return std::optional(RecordStream(*this, MinimumSupportedVersion, MockRecordHeader));
}

std::size_t Reader::read(std::vector<uint8_t>& buffer, std::size_t size, timeout_t timeout)
{
    return m_pimpl->read(buffer, size, timeout);
}

void Reader::stop_recording()
{
    m_pimpl->stop();
}

std::string Reader::get_port() const
{
    return "MOCKED";
}

std::optional<FirmwareVersion> Reader::query_version(timeout_t timeout)
{
    if (m_pimpl->m_state == Impl::State::Idle) {
        return std::make_optional(MockVersionInfo);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    return std::nullopt;
}

std::optional<std::string> Reader::query_build_id(timeout_t timeout)
{
    if (m_pimpl->m_state == Impl::State::Idle) {
        return std::make_optional(MockBuildDigest);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    return std::nullopt;
}

}; // namespace recap::reader
