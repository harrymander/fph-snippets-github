#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#if __clang__
#include <tuple>
#endif

#include "endian.hpp"

namespace recap::record {

static constexpr std::string_view RecordFileSignature = "-RECAP-";

using RecordMajorVersionFieldType = uint8_t;
using RecordMinorVersionFieldType = uint8_t;

// File size field is located after file signature and major, minor version bytes
static constexpr size_t RecordFileSizeFieldOffset = RecordFileSignature.size()
    + sizeof(RecordMajorVersionFieldType) + sizeof(RecordMinorVersionFieldType);
using RecordFileSizeFieldType = uint64_t;

// Sample offset field is located after file size field
static constexpr size_t RecordFileSampleOffsetFieldOffset =
    RecordFileSizeFieldOffset + sizeof(RecordFileSizeFieldType);
using RecordFileSampleOffsetFieldType = uint64_t;

static_assert(sizeof(RecordFileSizeFieldType) == 8);
static_assert(sizeof(RecordFileSampleOffsetFieldType) == 8);

struct InvalidRecordError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct FirmwareVersion {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
    std::optional<std::string> extra;

#ifdef __clang__
    // FIXME: clang doesn't seem to support std::string::operator<=>
    bool operator<(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) < std::tie(o.major, o.minor, o.patch, o.extra);
    }

    bool operator<=(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) <= std::tie(o.major, o.minor, o.patch, o.extra);
    }

    bool operator>(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) > std::tie(o.major, o.minor, o.patch, o.extra);
    }

    bool operator>=(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) >= std::tie(o.major, o.minor, o.patch, o.extra);
    }

    bool operator==(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) == std::tie(o.major, o.minor, o.patch, o.extra);
    }

    bool operator!=(const FirmwareVersion& o) const
    {
        return std::tie(major, minor, patch, extra) != std::tie(o.major, o.minor, o.patch, o.extra);
    }
#else
    auto operator<=>(const FirmwareVersion&) const = default;
#endif

    explicit FirmwareVersion(std::string_view string);

    constexpr FirmwareVersion(unsigned int major, unsigned int minor, unsigned int patch) :
        major(major), minor(minor), patch(patch), extra(std::nullopt)
    {}

    FirmwareVersion(
        unsigned int major, unsigned int minor, unsigned int patch, std::string_view extra
    );

    std::string str() const;
};

struct RecordVersion {
    RecordMajorVersionFieldType major;
    RecordMinorVersionFieldType minor;

    constexpr RecordVersion(decltype(major) major, decltype(minor) minor) :
        major(major), minor(minor)
    {}

    auto operator<=>(const RecordVersion& other) const = default;
};

inline std::ostream& operator<<(std::ostream& os, const RecordVersion& version)
{
    os << static_cast<unsigned int>(version.major) << '.'
       << static_cast<unsigned int>(version.minor);
    return os;
}

static constexpr recap::record::RecordVersion MinimumSupportedVersion(5, 0);
static constexpr recap::record::RecordVersion MaximumSupportedVersion(5, 0);

struct InvalidRecordHeaderError : public InvalidRecordError {
    using InvalidRecordError::InvalidRecordError;

    static InvalidRecordHeaderError from_invalid_line(std::string_view line);
    static InvalidRecordHeaderError from_missing_key(std::string_view key);
    static InvalidRecordHeaderError
    from_invalid_numeric(std::string_view numeric, std::errc err = {});
};

class Header {
public:
    using FieldsMap = std::unordered_map<std::string, std::string>;

    /**
     * Parses buffer and returns a new Header instance.
     *
     * Raises InvalidRecordError on error.
     */
    static Header from_buffer(const std::string& buffer);

    /**
     * For string fields, leading spaces of the value are removed. Field keys and values (after
     * leading space removal) may not be empty.
     *
     * Raises std::invalid_argument if any field keys or values are invalid.
     */
    Header(
        const FirmwareVersion& firmware_version,
        const std::string& build_digest,
        unsigned int sample_rate_khz,
        int adc_vref_millivolts,
        const std::string& board_name,
        const std::string& flow_meter_model,
        int flow_meter_offset,
        int flow_meter_scale,
        bool flow_meter_signed,
        const std::string& h2s_sensor_barcode,
        const FieldsMap& extra_fields = {}
    );

    std::string buffer_str() const;

    const FirmwareVersion& firmware_version() const;
    const std::string& build_digest() const;
    unsigned int sample_rate_khz() const;
    int adc_vref_millivolts() const;
    const std::string& board_name() const;
    const std::string& flow_meter_model() const;
    int flow_meter_offset() const;
    int flow_meter_scale() const;
    bool flow_meter_signed() const;
    const std::string& h2s_sensor_barcode() const;
    FieldsMap::size_type num_extra_fields() const;
    const FieldsMap& extra_fields() const;

    /**
     * Returns pointer to string for extra header field at `key`, or `nullptr` if the field doesn't
     * exist.
     */
    const std::string *get_extra_field(const std::string& key) const;

    /**
     * Overwrites an existing field if it already exists.
     */
    void set_extra_field(const std::string& key, const std::string& value);

    bool operator==(const Header& other) const = default;

private:
    FirmwareVersion m_firmware_version;
    std::string m_build_digest;
    unsigned int m_sample_rate;
    int m_adc_vref;
    std::string m_board_name;
    std::string m_flow_model;
    int m_flow_offset;
    int m_flow_scale;
    bool m_flow_signed;
    std::string m_h2s_sensor_barcode;
    FieldsMap m_extras;
};

struct Sample {
    virtual ~Sample() = default;
    Sample(const Sample&) = default;
    Sample(Sample&&) = default;
    Sample& operator=(const Sample&) = default;
    Sample& operator=(Sample&&) = default;

    virtual double millis() const = 0;
    virtual std::optional<double> flow_lpm() const = 0;
    virtual std::optional<double> pressure_pascals() const = 0;
    virtual double ear_volts() const = 0;
    virtual double mask_volts() const = 0;
    virtual uint8_t event() const = 0;
    virtual std::optional<int32_t> h2s_conc_ppb() const = 0;
    virtual std::optional<int8_t> h2s_temp_celsius() const = 0;
    virtual std::optional<uint8_t> h2s_humidity_percent() const = 0;

    const std::vector<uint8_t>& buffer() const { return m_buffer; }

    std::vector<uint8_t>& buffer() { return m_buffer; }

    std::vector<uint8_t>::size_type buffer_size() const { return m_buffer.size(); }

protected:
    explicit Sample(std::vector<uint8_t>&& buffer);

    template <class R, class... Previous> R get_raw() const
    {
        return endian::get_buffer_le<R>(m_buffer.data() + (0 + ... + sizeof(Previous)));
    }

private:
    std::vector<uint8_t> m_buffer;
};

std::ostream& operator<<(std::ostream& os, const Sample& sample);

struct CompleteSample final : public Sample {
    CompleteSample(std::vector<uint8_t>&& buffer, const Header& header);

    double millis() const override;
    std::optional<double> flow_lpm() const override;
    std::optional<double> pressure_pascals() const override;
    double ear_volts() const override;
    double mask_volts() const override;
    uint8_t event() const override;
    std::optional<int32_t> h2s_conc_ppb() const override;
    std::optional<int8_t> h2s_temp_celsius() const override;
    std::optional<uint8_t> h2s_humidity_percent() const override;

    using raw_millis_t = uint32_t;
    using raw_flow_t = uint16_t;
    using raw_pressure_t = int16_t;
    using raw_ear_t = uint16_t;
    using raw_mask_t = uint16_t;
    using raw_event_t = uint8_t;
    using raw_h2s_conc_t = int32_t;
    using raw_h2s_temp_t = int8_t;
    using raw_h2s_humidity_t = uint8_t;

    // clang-format off
    static constexpr size_t ExpectedBufferSize = (
        sizeof(raw_millis_t) +
        sizeof(raw_flow_t) +
        sizeof(raw_pressure_t) +
        sizeof(raw_ear_t) +
        sizeof(raw_mask_t) +
        sizeof(raw_event_t) +
        sizeof(raw_h2s_conc_t) +
        sizeof(raw_h2s_temp_t) +
        sizeof(raw_h2s_humidity_t)
    );
    // clang-format on

    static constexpr auto H2SMissingValue = std::numeric_limits<raw_h2s_conc_t>::min();

private:
    const Header& m_header;
};

class CompleteSampleWriter {
private:
    const Header& m_header;
    uint8_t *m_buffer;

    template <class R, class... Previous> inline CompleteSampleWriter& set_raw(R val)
    {
        endian::set_buffer_le(m_buffer + (0 + ... + sizeof(Previous)), val);
        return *this;
    }

    CompleteSampleWriter& set_flow_raw(CompleteSample::raw_flow_t val);
    CompleteSampleWriter& set_pressure_raw(CompleteSample::raw_pressure_t val);
    CompleteSampleWriter& set_ear_raw(CompleteSample::raw_ear_t val);
    CompleteSampleWriter& set_mask_raw(CompleteSample::raw_mask_t v);
    CompleteSampleWriter& set_h2s_conc_raw(CompleteSample::raw_h2s_conc_t val);
    CompleteSampleWriter& set_h2s_conc_raw_opt(std::optional<CompleteSample::raw_h2s_conc_t> val);
    CompleteSampleWriter& set_h2s_temp_raw(CompleteSample::raw_h2s_temp_t val);
    CompleteSampleWriter& set_h2s_humidity_raw(CompleteSample::raw_h2s_humidity_t val);

public:
    static constexpr std::size_t ExpectedBufferSize = CompleteSample::ExpectedBufferSize;

    CompleteSampleWriter(const Header& header, uint8_t *buffer);

    virtual ~CompleteSampleWriter() = default;
    CompleteSampleWriter(const CompleteSampleWriter&) = default;
    CompleteSampleWriter(CompleteSampleWriter&&) = default;
    CompleteSampleWriter& operator=(const CompleteSampleWriter&) = delete;
    CompleteSampleWriter& operator=(CompleteSampleWriter&&) = delete;

    CompleteSampleWriter& set_millis(CompleteSample::raw_millis_t millis);
    CompleteSampleWriter& set_flow_lpm(double);
    CompleteSampleWriter& set_pressure_pascals(double);
    CompleteSampleWriter& set_ear_volts(double);
    CompleteSampleWriter& set_mask_volts(double);
    CompleteSampleWriter& set_event(uint8_t);
    CompleteSampleWriter& clear_h2s();
    CompleteSampleWriter& set_h2s_conc_ppb(std::optional<int32_t>);
    CompleteSampleWriter& set_h2s_temp_celsius(int8_t);
    CompleteSampleWriter& set_h2s_humidity_percent(uint8_t);
};

struct AudioSample final : public Sample {
    AudioSample(std::vector<uint8_t>&& buffer, const Header& header, double millis, uint8_t event);

    double ear_volts() const override;
    double mask_volts() const override;
    uint8_t event() const override;

    double millis() const override { return m_millis; };

    std::optional<double> flow_lpm() const override { return std::nullopt; }

    std::optional<double> pressure_pascals() const override { return std::nullopt; }

    std::optional<int32_t> h2s_conc_ppb() const override { return std::nullopt; }

    std::optional<int8_t> h2s_temp_celsius() const override { return std::nullopt; }

    std::optional<uint8_t> h2s_humidity_percent() const override { return std::nullopt; }

    static constexpr size_t ExpectedBufferSize =
        sizeof(CompleteSample::raw_ear_t) + sizeof(CompleteSample::raw_mask_t);

private:
    const Header& m_header;
    double m_millis;
    uint8_t m_event;
};

class AudioSampleWriter {
private:
    const Header& m_header;
    uint8_t *m_buffer;

    template <class R, class... Previous> inline AudioSampleWriter& set_raw(R val)
    {
        endian::set_buffer_le(m_buffer + (0 + ... + sizeof(Previous)), val);
        return *this;
    }

    AudioSampleWriter& set_ear_raw(CompleteSample::raw_ear_t val);
    AudioSampleWriter& set_mask_raw(CompleteSample::raw_mask_t v);

public:
    static constexpr std::size_t ExpectedBufferSize = AudioSample::ExpectedBufferSize;

    AudioSampleWriter(const Header& header, uint8_t *buffer);

    virtual ~AudioSampleWriter() = default;
    AudioSampleWriter(const AudioSampleWriter&) = default;
    AudioSampleWriter(AudioSampleWriter&&) = default;
    AudioSampleWriter& operator=(const AudioSampleWriter&) = delete;
    AudioSampleWriter& operator=(AudioSampleWriter&&) = delete;

    AudioSampleWriter& set_ear_volts(double);
    AudioSampleWriter& set_mask_volts(double);
};

template <typename Writer> class SampleBuilder final : public Writer {
public:
    explicit SampleBuilder(const Header& header) : Writer(header, m_buffer.data()) {}

    using BufferArray = std::array<uint8_t, Writer::ExpectedBufferSize>;

    const BufferArray& buffer() const { return m_buffer; }

    BufferArray& buffer() { return m_buffer; }

private:
    BufferArray m_buffer{};
};

using CompleteSampleBuilder = SampleBuilder<CompleteSampleWriter>;
using AudioSampleBuilder = SampleBuilder<AudioSampleWriter>;

}; // namespace recap::record
