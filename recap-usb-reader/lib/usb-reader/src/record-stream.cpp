#include "reader.hpp"
#include "record.hpp"

#include <sstream>
#include <stdexcept>

namespace recap::reader {

using namespace recap::record;

RecordStream::RecordStream(Reader& reader, const RecordVersion& version, const Header& header) :
    m_reader(reader),
    m_version(version),
    m_header(header),
    m_ticks_per_milli(header.sample_rate_khz()),
    m_ticks(m_ticks_per_milli)
{}

const RecordVersion& RecordStream::version() const
{
    return m_version;
}

const Header& RecordStream::header() const
{
    return m_header;
}

bool RecordStream::read_into_buffer(std::size_t expected_size, Reader::timeout_t timeout)
{
    if (m_windex < expected_size) {
        const std::size_t to_read = expected_size - m_windex;
        const std::size_t r = m_reader.read(m_buffer, to_read, timeout);
        m_windex += r;
        if (r < to_read) {
            return false;
        }
    }

    m_windex = 0;
    return true;
}

RecordStream::ReadSampleResult
RecordStream::read_audio_sample(std::unique_ptr<Sample>& sample, Reader::timeout_t timeout)
{
    using enum ReadSampleResult;

    if (!read_into_buffer(AudioSample::ExpectedBufferSize, timeout)) {
        return Timeout;
    }

    sample = std::make_unique<AudioSample>(
        std::move(m_buffer), m_header, m_millis + double(m_ticks) / m_ticks_per_milli, m_last_event
    );
    m_buffer.clear(); // value of moved-from vector is valid but unspecified, so clear explicitly

    m_ticks++;
    return Ok;
}

RecordStream::ReadSampleResult
RecordStream::read_complete_sample(std::unique_ptr<Sample>& sample, Reader::timeout_t timeout)
{
    using enum ReadSampleResult;

    if (!read_into_buffer(CompleteSample::ExpectedBufferSize, timeout)) {
        return Timeout;
    }

    sample = std::make_unique<CompleteSample>(std::move(m_buffer), m_header);
    m_buffer.clear(); // value of moved-from vector is valid but unspecified, so clear explicitly
    if (m_event_override) {
        CompleteSampleWriter(m_header, sample->buffer().data()).set_event(1 << 1);
    }

    m_last_event = sample->event();

    const auto new_millis = sample->millis();
    if (new_millis == 0) {
        return Sentinel;
    }

    m_millis += 1;
    if (new_millis != m_millis) {
        std::stringstream ss;
        ss << "Got sample number out of order (expected " << m_millis << ", got " << new_millis
           << ")";
        throw std::runtime_error(ss.str());
    }

    m_ticks = 1;
    return Ok;
}

RecordStream::ReadSampleResult
RecordStream::read_sample(std::unique_ptr<Sample>& sample, Reader::timeout_t timeout)
{
    if (m_ticks < m_ticks_per_milli) {
        return read_audio_sample(sample, timeout);
    }

    return read_complete_sample(sample, timeout);
}

}; // namespace recap::reader
