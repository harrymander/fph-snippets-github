#include "writer.hpp"

#include "endian.hpp"
#include "record.hpp"

#include <ios>
#include <ostream>
#include <stdexcept>

namespace recap::writer {

using namespace recap::record;

RecordWriter::RecordWriter(
    std::ostream& os,
    const recap::record::RecordVersion& version,
    const recap::record::Header& header
) :
    os(os),
    m_sample_size(
        CompleteSample::ExpectedBufferSize
        + (header.sample_rate_khz() - 1) * AudioSample::ExpectedBufferSize
    ),
    m_start_pos(tellp())
{
    write_signature(version);
    write_header(header);
}

RecordWriter::~RecordWriter()
{
    try {
        finish();
    } catch (...) {
    }
}

bool RecordWriter::finished() const
{
    return m_finished;
}

void RecordWriter::write(const void *data, std::size_t count)
{
    os.write(static_cast<const char *>(data), count);
    if (os.fail()) {
        throw std::runtime_error("error writing to record output stream");
    }
    m_written += count;
}

void RecordWriter::write_signature(const RecordVersion& version)
{
    const std::array<
        uint8_t,
        2 + sizeof(RecordFileSizeFieldType) + sizeof(RecordFileSampleOffsetFieldType)>
        fields = {version.major, version.minor};

    write(RecordFileSignature.data(), RecordFileSignature.size());
    write(fields.data(), fields.size());
}

void RecordWriter::write_header(const Header& header)
{
    // Header is just the UTF-8 encoded header followed by a nul-char. Assuming UTF-8... probably
    // fine
    const auto buf = header.buffer_str();
    write(buf.c_str(), buf.size() + 1); // include the null-terminator
    m_sample_offset = m_written;
}

void RecordWriter::write_sample(const void *data, std::size_t size)
{
    /**
     * For an audio sample rate of f kHz, samples must always be written in groups starting with a
     * CompleteSample followed by (f - 1) AudioSamples. Buffer the sample writes so that if there is
     * an error in reading the samples, the written record file will still be valid when this object
     * is destroyed (some samples will be lost if this is the case).
     */

    const auto *buf = static_cast<const uint8_t *>(data);
    while (size) {
        const std::size_t to_write = std::min(m_sample_size - m_sample_buffer.size(), size);
        m_sample_buffer.insert(m_sample_buffer.end(), buf, buf + to_write);
        size -= to_write;
        buf += to_write;

        if (m_sample_buffer.size() == m_sample_size) {
            write(m_sample_buffer.data(), m_sample_buffer.size());
            m_sample_buffer.clear();
        }
    }
}

void RecordWriter::seekp(size_t off, std::ios_base::seekdir dir)
{
    os.seekp(off, dir);
    if (os.fail()) {
        throw std::runtime_error("error setting file position indicator");
    }
}

std::size_t RecordWriter::tellp()
{
    const std::size_t ret = os.tellp();
    if (os.fail()) {
        throw std::runtime_error("error getting file position indicator");
    }
    return ret;
}

void RecordWriter::do_finish()
{
    // Write sentinel
    static const uint8_t zero_buffer[CompleteSample::ExpectedBufferSize] = {0};
    write(zero_buffer, sizeof(zero_buffer));

    const auto end_pos = tellp();

    // Seek back to near the start of the file and write the file size and sample offset fields
    seekp(m_start_pos + RecordFileSizeFieldOffset, std::ios_base::beg);
    const RecordFileSizeFieldType lesize = endian::htole(m_written);
    write(&lesize, sizeof(lesize));

    seekp(m_start_pos + RecordFileSampleOffsetFieldOffset, std::ios_base::beg);
    const RecordFileSampleOffsetFieldType leoffset = endian::htole(m_sample_offset);
    write(&leoffset, sizeof(leoffset));

    // Return back to end of stream
    seekp(end_pos, std::ios_base::beg);

    // Drop any incomplete samples
    m_sample_buffer.clear();
}

void RecordWriter::finish()
{
    if (!finished()) {
        do_finish();
    }

    m_finished = true;
}

}; // namespace recap::writer
