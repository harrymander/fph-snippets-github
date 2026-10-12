// Disable warnings about gmtime and strerror on MSVC
#define _CRT_SECURE_NO_WARNINGS

#include <recap/usb-reader/reader.hpp>
#include <recap/usb-reader/writer.hpp>
#include <utils/datetimestr.hpp>

#include <argparse/argparse.hpp>

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#ifdef __unix__
#include <csignal>

// Use sig_atomic_t since std::atomic may not be lock-free
static volatile sig_atomic_t g_interrupted = 0;

static void sigint_handler(int signum)
{
    if (signum == SIGINT) {
        g_interrupted = 1;
    }
}

static void init_interrupt()
{
    struct sigaction action = {};
    action.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &action, nullptr) < 0) {
        std::cerr << "Error setting sigaction\n";
        std::exit(EXIT_FAILURE);
    }
}
#else
#include <ConsoleApi.h>
#include <Windows.h>

#include <atomic>

static std::atomic_bool g_interrupted = false;

BOOL WINAPI console_ctrl_handler(_In_ DWORD control_type)
{
    switch (control_type) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        g_interrupted = true;
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

static void init_interrupt()
{
    if (!SetConsoleCtrlHandler(console_ctrl_handler, TRUE)) {
        std::cerr << "Error setting console control handler\n";
        std::exit(EXIT_FAILURE);
    }
}
#endif

using namespace recap;
using namespace std::string_literals;

static void parse_args(argparse::ArgumentParser& program, int argc, char const *argv[])
{
    program.add_argument("-d", "--device")
        .help("USB device path; if not provided will attempt to automatically "
              "detect device")
        .metavar("PATH");
    program.add_argument("-o", "--output").help("output file path").metavar("PATH");
    program.add_argument("-q", "--quiet")
        .help("do not print samples to stderr")
        .implicit_value(true)
        .default_value(false);
    program.add_argument("-n", "--millis")
        .help("approximate number of milliseconds to read. If 0 (the default),"
              " runs indefinitely. Note time is approximate and"
              " may be particularly inaccurate at low values.")
        .metavar("SAMPLES")
        .action([](const std::string& s) -> unsigned int {
            unsigned int val;
            const auto *const str_end = s.data() + s.size();
            const auto [ptr, ec] = std::from_chars(s.data(), str_end, val, 10);
            if (ptr != str_end || ec == std::errc::invalid_argument) {
                throw std::runtime_error("--millis/-n: invalid integer value: "s + s);
            }
            if (ec == std::errc::result_out_of_range) {
                throw std::runtime_error("--millis/-n: value out of range");
            }
            return val;
        });
    program.add_argument("--device-info")
        .help("equivalent to passing both --firmware-version --build-id")
        .implicit_value(true)
        .default_value(false);
    program.add_argument("--firmware-version")
        .help("queries the firmware version from device and prints it")
        .implicit_value(true)
        .default_value(false);
    program.add_argument("--build-id")
        .help("queries the build ID from device and prints it")
        .implicit_value(true)
        .default_value(false);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        std::exit(1);
    }
}

static void read_and_output_samples(
    reader::Reader& reader,
    std::optional<std::ofstream>& output,
    unsigned int num_samples,
    bool quiet
)
{
    auto recording_opt = reader.start_recording();
    if (!recording_opt) {
        throw std::runtime_error("Timed-out starting recording");
    }
    auto recording = *recording_opt;

    auto header = recording.header();
    header.set_extra_field("Host", BUILD_INFO);
    header.set_extra_field("Datetime", utils::utc_datetime_str());
    std::unique_ptr<writer::RecordWriter> writer;
    if (output.has_value()) {
        writer = std::make_unique<writer::RecordWriter>(*output, recording.version(), header);
    }

    if (!quiet) {
        std::cout << "Record version: " << recording.version() << '\n';
        std::cout << header.buffer_str() << '\n';
    }

    bool stopping = false;
    bool stopped = false;
    int timeout = 1000;
    while (!stopped) {
        std::unique_ptr<record::Sample> sample;
        switch (recording.read_sample(sample, timeout)) {
            using enum reader::RecordStream::ReadSampleResult;
        case Ok: {
            if (!stopping && (g_interrupted || (num_samples && sample->millis() >= num_samples))) {
                stopping = true;
                reader.stop_recording();
            }
            if (writer) {
                writer->write_sample(sample->buffer().data(), sample->buffer_size());
            }
            if (!quiet) {
                std::cout << "Sample: " << *sample << "\n";
            }
            timeout = 0;
            break;
        }
        case Sentinel:
            stopped = true;
            break;
        case Timeout:
            if (timeout) {
                throw std::runtime_error("Timed-out reading sample");
            } else {
                timeout = 1000;
            }
            break;
        }
    }
}

template <class F> static std::ofstream open_write(F&& f)
{
    std::ofstream stream = f();
    if (stream.fail()) {
        throw std::runtime_error("cannot open file for writing: "s + std::strerror(errno));
    }
    return stream;
}

static int main_(const argparse::ArgumentParser& program)
{
    const auto output_path = program.present("--output");
    auto output = output_path.has_value() ? std::make_optional(open_write([&program]() {
        return std::ofstream(program.get("--output"), std::ios::binary);
    })) :
                                            std::nullopt;

    reader::Reader reader(program.present("--device"));

    const bool get_device_info = program.get<bool>("--device-info");
    const bool get_firmware_version = get_device_info || program.get<bool>("--firmware-version");
    const bool get_build_id = get_device_info || program.get<bool>("--build-id");
    if (get_firmware_version || get_build_id) {
        if (get_firmware_version) {
            const auto version = reader.query_version();
            if (!version) {
                throw std::runtime_error("timed-out getting version info");
            }
            std::cout << version->str() << '\n';
        }
        if (get_build_id) {
            const auto build_id = reader.query_build_id();
            if (!build_id) {
                throw std::runtime_error("timed-out getting build ID");
            }
            std::cout << *build_id << '\n';
        }
        return 0;
    }

    read_and_output_samples(
        reader,
        output,
        program.present<unsigned int>("--millis").value_or(0),
        program.get<bool>("--quiet")
    );
    return 0;
}

int main(int argc, char const *argv[])
{
    init_interrupt();
    argparse::ArgumentParser program(PROJECT_NAME, BUILD_INFO);
    parse_args(program, argc, argv);
    try {
        return main_(program);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
