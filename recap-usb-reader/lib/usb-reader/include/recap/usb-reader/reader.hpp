#pragma once

#include "record.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace recap::reader {

class RecordStream;

class Reader {
public:
    explicit Reader(std::optional<std::string> port = std::nullopt);
    ~Reader();
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
    Reader& operator=(Reader&&) = delete;
    Reader(Reader&&) = delete;

    using timeout_t = uint32_t;
    static constexpr timeout_t NoTimeout = std::numeric_limits<timeout_t>::max();

    std::optional<recap::record::FirmwareVersion> query_version(timeout_t timeout = 1000);
    std::optional<std::string> query_build_id(timeout_t timeout = 1000);
    std::string get_port() const;

    // Returns nullopt on timeout
    std::optional<RecordStream> start_recording();
    void stop_recording();

private:
    friend class RecordStream;

    // Timeout if return value is < size
    std::size_t read(std::vector<uint8_t>& buffer, std::size_t size, timeout_t timeout);

    struct Impl;
    std::unique_ptr<Impl> m_pimpl;
};

class RecordStream {
public:
    enum class ReadSampleResult {
        Ok,
        Sentinel,
        Timeout,
    };

    /**
     * Reads a sample from the stream and stores in sample. Sample pointer is only written to if
     * return value is not `ReadSampleResult::Timeout`.
     *
     * Returned sample is only valid with lifetime of this `RecordStream`.
     */
    ReadSampleResult
    read_sample(std::unique_ptr<recap::record::Sample>& sample, Reader::timeout_t timeout = 1000);

    void set_event_override(bool override) { m_event_override = override; }

    const recap::record::RecordVersion& version() const;
    const recap::record::Header& header() const;

private:
    ReadSampleResult
    read_complete_sample(std::unique_ptr<recap::record::Sample>& sample, Reader::timeout_t timeout);

    ReadSampleResult
    read_audio_sample(std::unique_ptr<recap::record::Sample>& sample, Reader::timeout_t timeout);

    bool read_into_buffer(std::size_t expected_size, Reader::timeout_t timeout);

    friend class Reader;

    RecordStream(
        Reader& reader,
        const recap::record::RecordVersion& version,
        const recap::record::Header& header
    );

    Reader& m_reader;
    recap::record::RecordVersion m_version;
    recap::record::Header m_header;

    recap::record::CompleteSample::raw_millis_t m_millis = 0;
    unsigned int m_ticks_per_milli;
    unsigned int m_ticks;
    uint8_t m_last_event = 0;
    bool m_event_override = false;

    std::vector<uint8_t> m_buffer;
    std::size_t m_windex = 0;
};

}; // namespace recap::reader
