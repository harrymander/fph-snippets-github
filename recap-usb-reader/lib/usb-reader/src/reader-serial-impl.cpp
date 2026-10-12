#include "reader.hpp"
#include "record.hpp"

#include <serial/serial.h>

#include <sys/types.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#define USB_HWID "USB\\VID_03EB&PID_6202&REV_0110"
#else
#define USB_HWID "USB VID:PID=03eb:6202"
#endif

namespace recap::reader {

using namespace std::string_literals;
using namespace recap::record;

class ReaderThread {
public:
    explicit ReaderThread(serial::Serial& serial) :
        m_serial(serial), m_timeout_obj(serial::Timeout::simpleTimeout(0))
    {
        try {
            serial.setTimeout(m_timeout_obj);
        } catch (const serial::IOException& e) {
            throw std::runtime_error("error setting serial timeout: "s + e.what());
        } catch (const serial::SerialException& e) {
            throw std::runtime_error("error setting serial timeout: "s + e.what());
        }

        m_thread = std::thread([this]() { runner(); });
    }

    ReaderThread(const ReaderThread&) = delete;
    ReaderThread(ReaderThread&&) = delete;
    ReaderThread& operator=(const ReaderThread&) = delete;
    ReaderThread& operator=(ReaderThread&&) = delete;

    ~ReaderThread()
    {
        m_stop = true;
        m_thread.join();

        if (m_serial.isOpen()) {
            /**
             * A bit of a hack - the Serial dtor calls Serial::close, which may throw
             * serial::IOException, so catch and ignore it here so this dtor doesn't throw.
             */
            try {
                m_serial.close();
            } catch (const serial::IOException&) {
            }
        }
    }

    std::size_t read(uint8_t *dest, std::size_t size, Reader::timeout_t timeout)
    {
        check_error();

        if (timeout == 0) {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            if (m_queue.empty()) {
                return 0;
            }
            return read_from_queue(dest, size);
        }

        using Clock = std::chrono::steady_clock;
        std::size_t numread = 0;
        std::unique_lock<std::mutex> lock(m_queue_mutex); // (locks mutex)
        const auto wait_end = Clock::now() + std::chrono::milliseconds(timeout);

        // (mutex must be locked on call to wait_until otherwise UB)
        while (m_notifier.wait_until(lock, wait_end, [this]() { return this->m_queue.size() > 0; }))
        {
            numread += read_from_queue(dest + numread, size - numread);
            if (numread == size) {
                break;
            }
        }
        return numread;
    }

    std::size_t available() const
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return m_queue.size();
    }

private:
    // Reads at most size bytes and returns the number of bytes read; queue mutex must be acquired.
    std::size_t read_from_queue(uint8_t *dest, std::size_t size)
    {
        const std::size_t to_read = std::min(size, m_queue.size());
        for (std::size_t i = 0; i < to_read; i++) {
            dest[i] = m_queue.front();
            m_queue.pop();
        }
        return to_read;
    }

    void check_error()
    {
        std::lock_guard<std::mutex> lock(m_error_mutex);
        if (m_error) {
            throw *m_error;
        }
    }

    std::size_t serial_read(uint8_t *dest, std::size_t size)
    {
        try {
            return m_serial.read(dest, size);
        } catch (const serial::IOException& e) {
            throw std::runtime_error("read error: "s + e.what());
        } catch (const serial::SerialException& e) {
            throw std::runtime_error("read error: "s + e.what());
        }
    }

    void runner()
    {
        while (!m_stop) {
            try {
                std::array<uint8_t, 1024> buffer;
                const auto numread = serial_read(buffer.data(), buffer.size());
                if (numread) {
                    {
                        std::lock_guard<std::mutex> lock(m_queue_mutex);
                        for (std::size_t i = 0; i < numread; i++) {
                            m_queue.push(buffer[i]);
                        }
                    }
                    m_notifier.notify_one();
                }
            } catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(m_error_mutex);
                m_error = e;
                m_stop = true;
            }
        }
    }

    serial::Serial& m_serial;
    serial::Timeout m_timeout_obj;
    std::thread m_thread;
    std::optional<std::exception> m_error;
    std::queue<uint8_t> m_queue;

    std::atomic<bool> m_stop = false;
    std::condition_variable m_notifier;
    mutable std::mutex m_queue_mutex;
    mutable std::mutex m_error_mutex;
};

static serial::Serial open_serial(const std::optional<std::string>& port)
{
    std::string path;
    if (port.has_value()) {
        path = port.value();
    } else {
        const auto ports = serial::list_ports();
        const auto info = std::find_if(ports.begin(), ports.end(), [](const auto& info) {
            return info.hardware_id.find(USB_HWID) != std::string::npos;
        });

        if (info == ports.end()) {
            throw std::runtime_error("Cannot find USB device");
        }

        path = info->port;
    }

    try {
        return serial::Serial(path);
    } catch (const serial::IOException& e) {
        throw std::runtime_error(e.what());
    }
}

struct Reader::Impl {
    explicit Impl(const std::optional<std::string> port) :
        m_ser(open_serial(port)), m_reader_thread(m_ser)
    {}

    enum class Command : char {
        START_RECORDING = 's',
        STOP_RECORDING = 'x',
        GET_BUILD_ID = 'b',
        GET_VERSION = 'v',
    };

    void write_cmd(Command cmd)
    {
        try {
            m_ser.write(reinterpret_cast<uint8_t *>(&cmd), 1);
        } catch (const serial::IOException& e) {
            throw std::runtime_error("write error: "s + e.what());
        } catch (const serial::SerialException& e) {
            throw std::runtime_error("write error: "s + e.what());
        }
    }

    /**
     * Read up to max_size stopping at newline, return nullopt if no newline before timeout or
     * max_size chars read
     */
    std::optional<std::string> readline(std::size_t max_size, Reader::timeout_t timeout)
    {
        /**
         * We aren't checking if input is valid UTF-8 string, but probably fine for just reading
         * the various strings in the header...
         */

        std::string str;
        for (std::size_t i = 0; i < max_size; i++) {
            uint8_t c;
            if (m_reader_thread.read(&c, 1, timeout) == 0) {
                return std::nullopt;
            }
            if (c == '\n') {
                return str;
            }
            str.push_back(c);
        }

        // If haven't read newline yet, consider it a timeout...
        return std::nullopt;
    }

    std::size_t read(std::vector<uint8_t>& buffer, std::size_t size, Reader::timeout_t timeout)
    {
        // Avoid resizing buffer if we are not waiting and there are no bytes ready to read
        if (timeout == 0 && m_reader_thread.available() == 0) {
            return 0;
        }

        const std::size_t original_size = buffer.size();
        buffer.resize(original_size + size);
        std::size_t r;
        try {
            r = m_reader_thread.read(buffer.data() + original_size, size, timeout);
        } catch (const std::runtime_error&) {
            buffer.resize(original_size);
            throw;
        }
        buffer.resize(original_size + r);
        return r;
    }

    std::string get_port() const { return m_ser.getPort(); }

    std::optional<RecordVersion> read_signature(timeout_t timeout)
    {
        std::array<uint8_t, RecordFileSignature.size()> signature = {};
        if (m_reader_thread.read(signature.data(), signature.size(), timeout) < signature.size()) {
            return std::nullopt;
        }
        if (std::memcmp(signature.data(), RecordFileSignature.data(), signature.size()) != 0) {
            throw std::runtime_error("invalid file signature");
        }

        // uint8_t(major), uint8_t(minor), uint64_t(size), uint64_t(offset)
        std::array<uint8_t, 2 + 16> fields;
        if (m_reader_thread.read(fields.data(), fields.size(), timeout) < fields.size()) {
            return std::nullopt;
        }

        const RecordVersion version(fields[0], fields[1]);
        if (version < MinimumSupportedVersion || version > MaximumSupportedVersion) {
            throw std::runtime_error("unsupported record version");
        }

        return version;
    }

    std::optional<Header> read_header(timeout_t timeout)
    {
        std::string buffer;
        while (true) {
            uint8_t c;
            if (m_reader_thread.read(&c, 1, timeout) == 0) {
                return std::nullopt;
            }
            if (!c) {
                break;
            }
            buffer.push_back(c);
        }

        return Header::from_buffer(std::move(buffer));
    }

private:
    serial::Serial m_ser;
    ReaderThread m_reader_thread;
};

Reader::Reader(std::optional<std::string> port) : m_pimpl(std::make_unique<Impl>(std::move(port)))
{}

Reader::~Reader() = default;

std::optional<RecordStream> Reader::start_recording()
{
    m_pimpl->write_cmd(Impl::Command::START_RECORDING);
    const auto version = m_pimpl->read_signature(1000);
    if (!version) {
        return std::nullopt;
    }
    const auto header = m_pimpl->read_header(1000);
    if (!header) {
        return std::nullopt;
    }
    return std::optional(RecordStream(*this, *version, *header));
}

void Reader::stop_recording()
{
    m_pimpl->write_cmd(Impl::Command::STOP_RECORDING);
}

std::optional<recap::record::FirmwareVersion> Reader::query_version(Reader::timeout_t timeout)
{
    m_pimpl->write_cmd(Impl::Command::GET_VERSION);
    const auto version_string = m_pimpl->readline(0xffff, timeout);
    if (version_string.has_value()) {
        return recap::record::FirmwareVersion(version_string.value());
    }
    return std::nullopt;
}

std::optional<std::string> Reader::query_build_id(Reader::timeout_t timeout)
{
    m_pimpl->write_cmd(Impl::Command::GET_BUILD_ID);
    return m_pimpl->readline(0xffff, timeout);
}

std::string Reader::get_port() const
{
    return m_pimpl->get_port();
}

std::size_t Reader::read(std::vector<uint8_t>& buffer, std::size_t size, timeout_t timeout)
{
    return m_pimpl->read(buffer, size, timeout);
}

}; // namespace recap::reader
