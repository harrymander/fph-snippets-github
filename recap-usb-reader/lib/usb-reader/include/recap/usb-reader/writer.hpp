#pragma once

#include "record.hpp"

#include <ios>
#include <ostream>
#include <vector>

namespace recap::writer {

class RecordWriter {
public:
    RecordWriter(
        std::ostream& os,
        const recap::record::RecordVersion& version,
        const recap::record::Header& header
    );
    ~RecordWriter();

    RecordWriter(const RecordWriter&) = delete;
    RecordWriter& operator=(const RecordWriter&) = delete;
    RecordWriter(RecordWriter&&) = delete;
    RecordWriter& operator=(RecordWriter&&) = delete;

    void write_sample(const void *data, std::size_t count);
    void finish();

    bool finished() const;

private:
    std::ostream& os;
    bool m_finished = false;
    recap::record::RecordFileSizeFieldType m_written = 0;
    recap::record::RecordFileSampleOffsetFieldType m_sample_offset = 0;
    const std::size_t m_sample_size;
    std::vector<uint8_t> m_sample_buffer;
    std::size_t m_start_pos;

    void write(const void *data, std::size_t count);
    void write_signature(const recap::record::RecordVersion&);
    void write_header(const recap::record::Header&);
    void do_finish();
    void seekp(size_t off, std::ios_base::seekdir dir);
    std::size_t tellp();
};

}; // namespace recap::writer
