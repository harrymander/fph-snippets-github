#include <recap/usb-reader/record.hpp>
#include <recap/usb-reader/writer.hpp>

#include <fstream>
#include <iostream>

using namespace recap::record;
using recap::writer::RecordWriter;

static void write_record(std::ofstream& os, unsigned int num_millis)
{
    constexpr unsigned int SampleRate = 10;
    static const Header header(
        FirmwareVersion(5, 0, 0, "test"),
        "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
        SampleRate,
        5000,
        "Test device",
        "Test flow meter",
        123456,
        -120,
        false,
        "H2S barcode 123",
        {{"Extra", "Hello, world!"}}
    );
    RecordWriter writer(os, RecordVersion(5, 0), header);

    for (unsigned int i = 0; i < num_millis; i++) {
        CompleteSampleBuilder complete_sample(header);
        complete_sample.set_millis(i + 1);
        writer.write_sample(complete_sample.buffer().data(), complete_sample.buffer().size());

        for (unsigned int j = 0; j < SampleRate - 1; j++) {
            AudioSampleBuilder audio_sample(header);
            writer.write_sample(audio_sample.buffer().data(), audio_sample.buffer().size());
        }
    }

    // Write incomplete samples, these should not be written to file
    CompleteSampleBuilder complete_sample(header);
    complete_sample.set_millis(num_millis);
    writer.write_sample(complete_sample.buffer().data(), complete_sample.buffer().size());
    AudioSampleBuilder audio_sample(header);
    writer.write_sample(audio_sample.buffer().data(), audio_sample.buffer().size());
}

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <outpath> <num-millis>\n";
        return 1;
    }

    const char *outpath = argv[1];
    int num_millis = std::atoi(argv[2]);
    if (num_millis <= 0) {
        std::cerr << "Error: num millis must be positive non-zero integer\n";
        return 1;
    }

    std::ofstream os(outpath, std::ios::binary);
    if (os.fail()) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    write_record(os, num_millis);
}
