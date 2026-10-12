#include "writer-observer.hpp"

#include <recap/usb-reader/record.hpp>
#include <recap/usb-reader/writer.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using recap::app::device::Device;
using recap::record::Header;
using recap::record::RecordVersion;
using recap::record::Sample;
using recap::writer::RecordWriter;

namespace recap::app::writer_observer {

class WriterObserver::Observer {
public:
    Observer(
        WriterObserver& writer_obs,
        std::ofstream&& stream,
        const RecordVersion& record_version,
        const Header& header
    ) :
        m_writer_obs(writer_obs),
        m_stream(std::move(stream)),
        m_record_writer(RecordWriter(m_stream, record_version, header)),
        m_new_sample_sub(writer_obs.m_device.subscribe_new_sample([this](const Sample *sample) {
            try {
                m_record_writer.write_sample(sample->buffer().data(), sample->buffer_size());
            } catch (const std::runtime_error& err) {
                m_writer_obs.error(err.what());
            }
        }))
    {}

private:
    WriterObserver& m_writer_obs;
    std::ofstream m_stream;
    RecordWriter m_record_writer;
    Device::NewSampleObservable::Observer m_new_sample_sub;
};

WriterObserver::WriterObserver(Device& device) :
    m_device(device),
    m_new_recording_sub(device.subscribe_new_recording([this](auto... args) {
        on_new_recording(args...);
    })),
    m_end_recording_sub(device.subscribe_end_recording([this]() { reset(); })),
    m_disconnected_sub(device.subscribe_disconnected([this](auto) { reset(); }))
{}

WriterObserver::~WriterObserver() = default;

const std::optional<std::filesystem::path>& WriterObserver::next_output_path() const
{
    return m_next_output_path;
}

void WriterObserver::set_next_output_path(const std::filesystem::path& path)
{
    m_next_output_path.emplace(path);
}

void WriterObserver::clear_next_output_path()
{
    m_next_output_path.reset();
}

WriterObserver::ErrorObservable::Observer
WriterObserver::subscribe_error(const ErrorObservable::Function& func)
{
    return m_error_obs.subscribe(func);
}

void WriterObserver::reset()
{
    m_observer.reset();
    clear_next_output_path();
}

void WriterObserver::error(const std::string& err)
{
    m_error_obs.notify(err);
    reset();
}

void WriterObserver::on_new_recording(const RecordVersion& record_version, const Header& header)
{
    if (!m_next_output_path.has_value()) {
        return;
    }

    std::ofstream stream(*m_next_output_path, std::ios::binary | std::ios::out);
    if (!stream.good()) {
        error("Error opening file for writing");
        return;
    }

    try {
        m_observer = std::make_unique<Observer>(*this, std::move(stream), record_version, header);
    } catch (const std::runtime_error& err) {
        error(err.what());
    }
}

}; // namespace recap::app::writer_observer
