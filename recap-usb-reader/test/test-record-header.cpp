#include <recap/usb-reader/record.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace recap::record;

#define TEST_RECORD_VERSION(name) TEST(TestRecordVersion, name)

TEST_RECORD_VERSION(ExactVersionsAreEqual)
{
    ASSERT_EQ(RecordVersion(5, 2), RecordVersion(5, 2));
}

TEST_RECORD_VERSION(MajorVersionNumberIsGreaterThan)
{
    ASSERT_GT(RecordVersion(5, 0), RecordVersion(4, 10));
}

TEST_RECORD_VERSION(MinorVersionNumberIsGreaterThan)
{
    ASSERT_GT(RecordVersion(5, 5), RecordVersion(5, 2));
}

TEST_RECORD_VERSION(ValidHeaderStringWithoutExtraParses)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n" // test no space after colon
        "Sample-Rate: 100\n"
        "VREF:      5000\n" // test extra spaces after colon are ignored
        "Board:     v3\n" // test extra spaces after colon are ignored
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: 0123456\n" // test leading zero interpreted as decimal
        "Flow-Scale: -120\n"
        "Flow-Sign: 0\n"
        "H2S-Barcode: H2S barcode 123    \n" // test trailing spaces not ignored
    );
    // clang-format on

    const auto header = Header::from_buffer(buffer);

    ASSERT_EQ(header.firmware_version(), FirmwareVersion(5, 0, 0, "test"));
    ASSERT_EQ(header.build_digest(), "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef");
    ASSERT_EQ(header.sample_rate_khz(), 100);
    ASSERT_EQ(header.adc_vref_millivolts(), 5000);
    ASSERT_EQ(header.board_name(), "v3");
    ASSERT_EQ(header.flow_meter_model(), "SFM3003-300-CE (air)");
    ASSERT_EQ(header.flow_meter_offset(), 123456);
    ASSERT_EQ(header.flow_meter_scale(), -120);
    ASSERT_FALSE(header.flow_meter_signed());
    ASSERT_EQ(header.h2s_sensor_barcode(), "H2S barcode 123    ");
    ASSERT_EQ(header.num_extra_fields(), 0);
    ASSERT_EQ(header.get_extra_field("Extra"), nullptr);
    ASSERT_EQ(header.get_extra_field("Firmware"), nullptr);
}

TEST_RECORD_VERSION(ValidHeaderStringWithNoTrailingNewlineParses)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 6\n"
        "VREF: 5000\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: 123456\n"
        "Flow-Scale: -120\n"
        "H2S-Barcode: H2S barcode 123\n"
        "Flow-Sign: 5" // no trailing newline
    );
    // clang-format on

    const auto header = Header::from_buffer(buffer);

    ASSERT_EQ(header.firmware_version(), FirmwareVersion(5, 0, 0, "test"));
    ASSERT_EQ(header.build_digest(), "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef");
    ASSERT_EQ(header.sample_rate_khz(), 6);
    ASSERT_EQ(header.adc_vref_millivolts(), 5000);
    ASSERT_EQ(header.board_name(), "v3");
    ASSERT_EQ(header.flow_meter_model(), "SFM3003-300-CE (air)");
    ASSERT_EQ(header.flow_meter_offset(), 123456);
    ASSERT_EQ(header.flow_meter_scale(), -120);
    ASSERT_TRUE(header.flow_meter_signed());
    ASSERT_EQ(header.h2s_sensor_barcode(), "H2S barcode 123");
    ASSERT_EQ(header.num_extra_fields(), 0);
    ASSERT_EQ(header.get_extra_field("Extra"), nullptr);
    ASSERT_EQ(header.get_extra_field("Firmware"), nullptr);
}

TEST_RECORD_VERSION(ValidHeaderStringWithExtraParses)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 1\n"
        "VREF: 4096\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
        "Extra 1: Hello, world!\n"
        "Extra 2 : Goodbye!\n"
        " Extra 3 : Goodbye!\n"
        " Extra 4: Goodbye!\n"
        " : Empty!\n"
    );
    // clang-format on

    const auto header = Header::from_buffer(buffer);

    ASSERT_EQ(header.firmware_version(), FirmwareVersion(5, 0, 0, "test"));
    ASSERT_EQ(header.build_digest(), "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef");
    ASSERT_EQ(header.sample_rate_khz(), 1);
    ASSERT_EQ(header.adc_vref_millivolts(), 4096);
    ASSERT_EQ(header.board_name(), "v3");
    ASSERT_EQ(header.flow_meter_model(), "SFM3003-300-CE (air)");
    ASSERT_EQ(header.flow_meter_offset(), -123456);
    ASSERT_EQ(header.flow_meter_scale(), 120);
    ASSERT_TRUE(header.flow_meter_signed());
    ASSERT_EQ(header.h2s_sensor_barcode(), "H2S barcode 123");

    ASSERT_EQ(header.num_extra_fields(), 5);

    const auto extra1 = header.get_extra_field("Extra 1");
    ASSERT_NE(extra1, nullptr);
    ASSERT_EQ(*extra1, "Hello, world!");

    const auto extra2 = header.get_extra_field("Extra 2 ");
    ASSERT_NE(extra2, nullptr);
    ASSERT_EQ(*extra2, "Goodbye!");

    const auto extra3 = header.get_extra_field(" Extra 3 ");
    ASSERT_NE(extra3, nullptr);
    ASSERT_EQ(*extra3, "Goodbye!");

    const auto extra4 = header.get_extra_field(" Extra 4");
    ASSERT_NE(extra4, nullptr);
    ASSERT_EQ(*extra4, "Goodbye!");

    const auto extra5 = header.get_extra_field(" ");
    ASSERT_NE(extra5, nullptr);
    ASSERT_EQ(*extra5, "Empty!");

    ASSERT_EQ(header.get_extra_field("Firmware"), nullptr);
    ASSERT_EQ(header.get_extra_field("does not exist"), nullptr);
}

TEST_RECORD_VERSION(ParsingEmptyStringThrows)
{
    ASSERT_THROW(Header::from_buffer({}), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingEmptyFieldKeyThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 4096\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
        ": value\n" // no key
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingEmptyFieldValueThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 4096\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
        "Empty:\n" // no value
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingWhitespaceOnlyFieldValueThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 4096\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
        "Spaces:   \n" // only spaces for value
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingEmptyFieldKeyAndValueThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build:deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 4096\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
        ":\n" // no key or value
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingMissingFieldThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build: deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 4096\n"
        "Board: v3\n"
        // missing "Flow" field
        "Flow-Offset: -123456\n"
        "Flow-Scale: 120\n"
        "Flow-Sign: 1\n"
        "H2S-Barcode: H2S barcode 123\n"
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingInvalidNumericThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build: deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: abc\n" // invalid numeric
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: 123456\n"
        "Flow-Scale: -120\n"
        "Flow-Sign: 0\n"
        "H2S-Barcode: H2S barcode 123\n"
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingMissingColonThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "Build: deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "Sample-Rate: 5\n"
        "VREF: 5000\n"
        "Board v3\n" // missing colon
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: 123456\n"
        "Flow-Scale: -120\n"
        "Flow-Sign: 0\n"
        "H2S-Barcode: H2S barcode 123\n"
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(ParsingEmptyLineThrows)
{
    // clang-format off
    const std::string buffer(
        "Firmware: 5.0.0-test\n"
        "\n" // empty line
        "Sample-Rate: 5\n"
        "Build: deadbeefdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "VREF: 5000\n"
        "Board: v3\n"
        "Flow: SFM3003-300-CE (air)\n"
        "Flow-Offset: 123456\n"
        "Flow-Scale: -120\n"
        "Flow-Sign: 0\n"
        "H2S-Barcode: H2S barcode 123\n"
    );
    // clang-format on

    ASSERT_THROW(Header::from_buffer(buffer), InvalidRecordHeaderError);
}

TEST_RECORD_VERSION(EmptyExtraFieldKeyThrows)
{
    ASSERT_THROW(
        Header(
            FirmwareVersion(5, 0, 0, "test"),
            "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
            10,
            5000,
            "v3",
            "SFM3003-300-CE",
            12345,
            -100,
            false,
            "Barcode 123",
            {{"", "Empty key"}}
        ),
        std::invalid_argument
    );
}

TEST_RECORD_VERSION(EmptyExtraFieldValueThrows)
{
    ASSERT_THROW(
        Header(
            FirmwareVersion(5, 0, 0, "test"),
            "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
            10,
            5000,
            "v3",
            "SFM3003-300-CE",
            12345,
            -100,
            false,
            "Barcode 123",
            {{"Empty value", ""}}
        ),
        std::invalid_argument
    );
}

TEST_RECORD_VERSION(WhitespaceOnlyExtraFieldValueThrows)
{
    ASSERT_THROW(
        Header(
            FirmwareVersion(5, 0, 0, "test"),
            "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
            10,
            5000,
            "v3",
            "SFM3003-300-CE",
            12345,
            -100,
            false,
            "Barcode 123",
            {{"Whitespace", "   "}}
        ),
        std::invalid_argument
    );
}

TEST_RECORD_VERSION(ConvertsToAndFromBuffer)
{
    const Header header(
        FirmwareVersion(5, 0, 0, "test"),
        "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
        8,
        5000,
        "v3",
        "SFM3003-300-CE",
        12345,
        -100,
        false,
        "Barcode 123",
        {{"abc", "def"}, {"123", "456"}}
    );

    const auto buffer = header.buffer_str();
    ASSERT_EQ(header, Header::from_buffer(buffer));
}

TEST_RECORD_VERSION(ConvertsToAndFromBufferWithWhitespaceInStrings)
{
    const Header header(
        FirmwareVersion(5, 0, 0, "test"),
        "   deadbeefdeadbeefdeadbeefdeadbeefdeadbeef ",
        7,
        5000,
        "  \tv3   ",
        "   SFM3003-300-CE ",
        12345,
        -100,
        false,
        " Barcode 123",
        {
            {"extra1 ", "value1 "},
            {" extra2", " value2"},
            {" extra3 ", " value3 "},
        }
    );

    ASSERT_EQ(header.build_digest(), "deadbeefdeadbeefdeadbeefdeadbeefdeadbeef ");
    ASSERT_EQ(header.board_name(), "\tv3   ");
    ASSERT_EQ(header.flow_meter_model(), "SFM3003-300-CE ");
    ASSERT_EQ(header.h2s_sensor_barcode(), "Barcode 123");
    ASSERT_EQ(header.num_extra_fields(), 3);
    ASSERT_EQ(*header.get_extra_field("extra1 "), "value1 ");
    ASSERT_EQ(*header.get_extra_field(" extra2"), "value2");
    ASSERT_EQ(*header.get_extra_field(" extra3 "), "value3 ");

    const auto buffer = header.buffer_str();
    ASSERT_EQ(header, Header::from_buffer(buffer));
}
