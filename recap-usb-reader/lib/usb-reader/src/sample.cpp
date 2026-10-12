#include "endian.hpp"
#include "record.hpp"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace recap::record {

std::ostream& operator<<(std::ostream& os, const Sample& sample)
{
    os << "time=" << sample.millis() << "[ms]";

    const auto flow = sample.flow_lpm();
    os << ", flow=";
    if (flow) {
        os << *flow << "[lpm]";
    } else {
        os << "n/a";
    }

    const auto pressure = sample.pressure_pascals();
    os << ", pressure=";
    if (pressure) {
        os << *pressure << "[Pa]";
    } else {
        os << "n/a";
    }

    os << ", ear=" << sample.ear_volts() << "[V]";
    os << ", mask=" << sample.mask_volts() << "[V]";

    os << ", h2s=";
    const auto h2s_ppb = sample.h2s_conc_ppb();
    if (h2s_ppb.has_value()) {
        os << (*h2s_ppb) << "[ppb] ";
    } else {
        os << "n/a";
    }

    os << ", h2s_temp=";
    const auto h2s_celsius = sample.h2s_temp_celsius();
    if (h2s_celsius.has_value()) {
        os << (*h2s_celsius) << "[°C] ";
    } else {
        os << "n/a";
    }

    os << ", h2s_humidity=";
    const auto h2s_humidity = sample.h2s_humidity_percent();
    if (h2s_humidity.has_value()) {
        os << (*h2s_humidity) << "[%] ";
    } else {
        os << "n/a";
    }

    os << ", event=" << (sample.event() ? "yes" : "no");
    return os;
}

Sample::Sample(std::vector<uint8_t>&& buffer) : m_buffer(std::move(buffer)) {}

CompleteSample::CompleteSample(std::vector<uint8_t>&& buffer, const Header& header) :
    Sample(std::move(buffer)), m_header(header)
{}

double CompleteSample::millis() const
{
    return get_raw<raw_millis_t>();
}

std::optional<double> CompleteSample::flow_lpm() const
{
    const auto raw = get_raw<raw_flow_t, raw_millis_t>();
    const double raw_double = m_header.flow_meter_signed() ? endian::signed_bit_cast(raw) : raw;
    return (raw_double - m_header.flow_meter_offset()) / m_header.flow_meter_scale();
}

std::optional<double> CompleteSample::pressure_pascals() const
{
    return static_cast<double>(get_raw<raw_pressure_t, raw_millis_t, raw_flow_t>()) / 60.0;
}

double CompleteSample::ear_volts() const
{
    const auto raw =
        static_cast<double>(get_raw<raw_ear_t, raw_millis_t, raw_flow_t, raw_pressure_t>());
    return m_header.adc_vref_millivolts() * raw / std::numeric_limits<raw_ear_t>::max() / 1000.0;
}

double CompleteSample::mask_volts() const
{
    const auto raw = static_cast<double>(
        get_raw<raw_mask_t, raw_millis_t, raw_flow_t, raw_pressure_t, raw_ear_t>()
    );
    return m_header.adc_vref_millivolts() * raw / std::numeric_limits<raw_mask_t>::max() / 1000.0;
}

uint8_t CompleteSample::event() const
{
    return get_raw<raw_event_t, raw_millis_t, raw_flow_t, raw_pressure_t, raw_ear_t, raw_mask_t>();
}

std::optional<int32_t> CompleteSample::h2s_conc_ppb() const
{
    const auto val = get_raw<
        raw_h2s_conc_t,
        raw_millis_t,
        raw_flow_t,
        raw_pressure_t,
        raw_ear_t,
        raw_mask_t,
        raw_event_t>();
    return val == H2SMissingValue ? std::nullopt : std::make_optional(val);
}

std::optional<int8_t> CompleteSample::h2s_temp_celsius() const
{
    if (h2s_conc_ppb().has_value()) {
        return get_raw<
            raw_h2s_temp_t,
            raw_millis_t,
            raw_flow_t,
            raw_pressure_t,
            raw_ear_t,
            raw_mask_t,
            raw_event_t,
            raw_h2s_conc_t>();
    }

    return std::nullopt;
}

std::optional<uint8_t> CompleteSample::h2s_humidity_percent() const
{
    if (h2s_conc_ppb().has_value()) {
        return get_raw<
            raw_h2s_humidity_t,
            raw_millis_t,
            raw_flow_t,
            raw_pressure_t,
            raw_ear_t,
            raw_mask_t,
            raw_event_t,
            raw_h2s_conc_t,
            raw_h2s_temp_t>();
    }

    return std::nullopt;
}

AudioSample::AudioSample(
    std::vector<uint8_t>&& buffer, const Header& header, double millis, uint8_t event
) :
    Sample(std::move(buffer)), m_header(header), m_millis(millis), m_event(event)
{}

double AudioSample::ear_volts() const
{
    const auto raw = static_cast<double>(get_raw<CompleteSample::raw_ear_t>());
    return m_header.adc_vref_millivolts() * raw
        / std::numeric_limits<CompleteSample::raw_ear_t>::max() / 1000;
}

double AudioSample::mask_volts() const
{
    const auto raw =
        static_cast<double>(get_raw<CompleteSample::raw_mask_t, CompleteSample::raw_ear_t>());
    return m_header.adc_vref_millivolts() * raw
        / std::numeric_limits<CompleteSample::raw_mask_t>::max() / 1000;
}

uint8_t AudioSample::event() const
{
    return m_event;
}

CompleteSampleWriter::CompleteSampleWriter(const Header& header, uint8_t *buffer) :
    m_header(header), m_buffer(buffer)
{}

CompleteSampleWriter& CompleteSampleWriter::set_millis(CompleteSample::raw_millis_t val)
{
    return set_raw<CompleteSample::raw_millis_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_flow_raw(CompleteSample::raw_flow_t val)
{
    return set_raw<CompleteSample::raw_flow_t, CompleteSample::raw_millis_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_pressure_raw(CompleteSample::raw_pressure_t val)
{
    return set_raw<
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_ear_raw(CompleteSample::raw_ear_t val)
{
    return set_raw<
        CompleteSample::raw_ear_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_mask_raw(CompleteSample::raw_mask_t v)
{
    return set_raw<
        CompleteSample::raw_mask_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_ear_t>(v);
}

CompleteSampleWriter& CompleteSampleWriter::set_event(uint8_t val)
{
    return set_raw<
        CompleteSample::raw_event_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_ear_t,
        CompleteSample::raw_mask_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_h2s_conc_raw(CompleteSample::raw_h2s_conc_t val)
{
    return set_raw<
        CompleteSample::raw_h2s_conc_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_ear_t,
        CompleteSample::raw_mask_t,
        CompleteSample::raw_event_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::clear_h2s()
{
    return set_h2s_conc_raw(CompleteSample::H2SMissingValue);
}

CompleteSampleWriter&
CompleteSampleWriter::set_h2s_conc_raw_opt(std::optional<CompleteSample::raw_h2s_conc_t> val)
{
    if (val.has_value()) {
        return set_h2s_conc_raw(*val);
    }

    return clear_h2s();
}

CompleteSampleWriter& CompleteSampleWriter::set_h2s_temp_raw(CompleteSample::raw_h2s_temp_t val)
{
    return set_raw<
        CompleteSample::raw_h2s_temp_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_ear_t,
        CompleteSample::raw_mask_t,
        CompleteSample::raw_event_t,
        CompleteSample::raw_h2s_conc_t>(val);
}

CompleteSampleWriter&
CompleteSampleWriter::set_h2s_humidity_raw(CompleteSample::raw_h2s_humidity_t val)
{
    return set_raw<
        CompleteSample::raw_h2s_humidity_t,
        CompleteSample::raw_millis_t,
        CompleteSample::raw_flow_t,
        CompleteSample::raw_pressure_t,
        CompleteSample::raw_ear_t,
        CompleteSample::raw_mask_t,
        CompleteSample::raw_event_t,
        CompleteSample::raw_h2s_conc_t,
        CompleteSample::raw_h2s_temp_t>(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_flow_lpm(double val)
{
    static_assert(std::is_unsigned_v<CompleteSample::raw_flow_t>, "raw_flow_t must be unsigned");

    double raw_double =
        std::round(m_header.flow_meter_offset() + (val * m_header.flow_meter_scale()));

    if (m_header.flow_meter_signed()) {
        using signed_raw_flow_t = std::make_signed_t<CompleteSample::raw_flow_t>;
        auto raw_signed = static_cast<signed_raw_flow_t>(raw_double);
        return set_flow_raw(std::bit_cast<CompleteSample::raw_flow_t>(raw_signed));
    }

    assert(raw_double >= 0);
    return set_flow_raw(static_cast<CompleteSample::raw_flow_t>(raw_double));
}

CompleteSampleWriter& CompleteSampleWriter::set_pressure_pascals(double val)
{
    return set_pressure_raw(static_cast<CompleteSample::raw_pressure_t>(std::round(60 * val)));
}

CompleteSampleWriter& CompleteSampleWriter::set_ear_volts(double val)
{
    return set_ear_raw(static_cast<CompleteSample::raw_ear_t>(std::round(
        std::numeric_limits<CompleteSample::raw_ear_t>::max() * val / m_header.adc_vref_millivolts()
        * 1000
    )));
}

CompleteSampleWriter& CompleteSampleWriter::set_mask_volts(double val)
{
    return set_mask_raw(static_cast<CompleteSample::raw_mask_t>(std::round(
        std::numeric_limits<CompleteSample::raw_ear_t>::max() * val / m_header.adc_vref_millivolts()
        * 1000
    )));
}

CompleteSampleWriter& CompleteSampleWriter::set_h2s_conc_ppb(std::optional<int32_t> val)
{
    return set_h2s_conc_raw_opt(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_h2s_temp_celsius(int8_t val)
{
    return set_h2s_temp_raw(val);
}

CompleteSampleWriter& CompleteSampleWriter::set_h2s_humidity_percent(uint8_t val)
{
    return set_h2s_humidity_raw(val);
}

AudioSampleWriter::AudioSampleWriter(const Header& header, uint8_t *buffer) :
    m_header(header), m_buffer(buffer)
{}

AudioSampleWriter& AudioSampleWriter::set_ear_raw(CompleteSample::raw_ear_t val)
{
    return set_raw<CompleteSample::raw_ear_t>(val);
}

AudioSampleWriter& AudioSampleWriter::set_mask_raw(CompleteSample::raw_mask_t v)
{
    return set_raw<CompleteSample::raw_mask_t, CompleteSample::raw_ear_t>(v);
}

AudioSampleWriter& AudioSampleWriter::set_ear_volts(double val)
{
    return set_ear_raw(static_cast<CompleteSample::raw_ear_t>(std::round(
        std::numeric_limits<CompleteSample::raw_ear_t>::max() * val / m_header.adc_vref_millivolts()
        * 1000
    )));
}

AudioSampleWriter& AudioSampleWriter::set_mask_volts(double val)
{
    return set_mask_raw(static_cast<CompleteSample::raw_mask_t>(std::round(
        std::numeric_limits<CompleteSample::raw_ear_t>::max() * val / m_header.adc_vref_millivolts()
        * 1000
    )));
}

} // namespace recap::record
