#include <recap/usb-reader/record.hpp>

#include <gtest/gtest.h>

#include <random>
#include <vector>

using namespace recap::record;

#define TEST_RECORD_SAMPLE(name) TEST(TestRecordSample, name)

#define ASSERT_DOUBLE_NEAR(val1, val2) ASSERT_NEAR(val1, val2, 1e-2)

static const Header SFM3000Header(
    FirmwareVersion(5, 0, 0),
    "digest",
    1000,
    5000,
    "test",
    "SFM3000-200",
    32000,
    140,
    false,
    "H2S barcode"
);

TEST_RECORD_SAMPLE(EmptyBufferHasMillisIsZero)
{
    CompleteSample sample(
        std::vector<uint8_t>(CompleteSample::ExpectedBufferSize, 0), SFM3000Header
    );
    ASSERT_EQ(sample.millis(), 0);
}

TEST_RECORD_SAMPLE(SetAndGetPositiveUnits)
{
    CompleteSampleBuilder builder(SFM3000Header);

    builder.set_millis(123)
        .set_flow_lpm(5.6)
        .set_pressure_pascals(500.5)
        .set_ear_volts((double) 0x2345 / 0xFFFF)
        .set_mask_volts((double) 0x1234 / 0xFFFF)
        .set_event(8)
        .set_h2s_conc_ppb(45)
        .set_h2s_humidity_percent(50)
        .set_h2s_temp_celsius(23);

    CompleteSample sample({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_EQ(sample.millis(), 123);
    ASSERT_DOUBLE_NEAR(*sample.flow_lpm(), 5.6);
    ASSERT_DOUBLE_NEAR(*sample.pressure_pascals(), 500.5);
    ASSERT_DOUBLE_NEAR(sample.ear_volts(), (double) 0x2345 / 0xFFFF);
    ASSERT_DOUBLE_NEAR(sample.mask_volts(), (double) 0x1234 / 0xFFFF);
    ASSERT_EQ(sample.event(), 8);
    ASSERT_EQ(*sample.h2s_conc_ppb(), 45);
    ASSERT_EQ(*sample.h2s_humidity_percent(), 50);
    ASSERT_EQ(*sample.h2s_temp_celsius(), 23);
}

TEST_RECORD_SAMPLE(SetAndGetSFM3003CEAirFlow)
{
    static const Header SFM3003Header(
        FirmwareVersion(5, 0, 0),
        "digest",
        1000,
        5000,
        "test",
        "SFM3003-300-CE (air)",
        -12288,
        120,
        true,
        "H2S barcode"
    );

    CompleteSampleBuilder builder(SFM3003Header);
    builder.set_flow_lpm(299.25);
    CompleteSample sample1({builder.buffer().begin(), builder.buffer().end()}, SFM3003Header);
    ASSERT_DOUBLE_NEAR(*sample1.flow_lpm(), 299.25);

    builder.set_flow_lpm(-145.5);
    CompleteSample sample2({builder.buffer().begin(), builder.buffer().end()}, SFM3003Header);
    ASSERT_DOUBLE_NEAR(*sample2.flow_lpm(), -145.5);
}

TEST_RECORD_SAMPLE(SetAndGetNegativeUnits)
{
    // Only testing fields that can be negative
    CompleteSampleBuilder builder(SFM3000Header);
    builder.set_flow_lpm(-5.6)
        .set_pressure_pascals(-500.5)
        .set_h2s_conc_ppb(-45)
        .set_h2s_temp_celsius(-23);

    CompleteSample sample({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_DOUBLE_NEAR(*sample.flow_lpm(), -5.6);
    ASSERT_DOUBLE_NEAR(*sample.pressure_pascals(), -500.5);
    ASSERT_EQ(*sample.h2s_conc_ppb(), -45);
    ASSERT_EQ(*sample.h2s_temp_celsius(), -23);
}

TEST_RECORD_SAMPLE(ClearH2S)
{
    CompleteSampleBuilder builder(SFM3000Header);
    builder.set_h2s_conc_ppb(12).set_h2s_humidity_percent(23).set_h2s_temp_celsius(45);

    CompleteSample sample1({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_EQ(*sample1.h2s_conc_ppb(), 12);
    ASSERT_EQ(*sample1.h2s_humidity_percent(), 23);
    ASSERT_EQ(*sample1.h2s_temp_celsius(), 45);

    builder.clear_h2s();
    CompleteSample sample2({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_FALSE(sample2.h2s_conc_ppb().has_value());
    ASSERT_FALSE(sample2.h2s_temp_celsius().has_value());
    ASSERT_FALSE(sample2.h2s_humidity_percent().has_value());
}

TEST_RECORD_SAMPLE(SetAndGetEvent)
{
    CompleteSampleBuilder builder(SFM3000Header);
    builder.set_event(8);

    CompleteSample sample1({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_EQ(sample1.event(), 8);

    builder.set_event(0);
    CompleteSample sample2({builder.buffer().begin(), builder.buffer().end()}, SFM3000Header);
    ASSERT_EQ(sample2.event(), 0);
}
