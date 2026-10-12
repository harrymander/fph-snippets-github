#include <recap/usb-reader/reader.hpp>
#include <recap/usb-reader/record.hpp>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>

using recap::record::FirmwareVersion;

TEST(TestFirmwareVersion, WithoutNoExtraVersionInfo)
{
    ASSERT_EQ(FirmwareVersion("1.12.45"), FirmwareVersion(1, 12, 45));
}

TEST(TestFirmwareVersion, WithExtraVersionInfo)
{
    ASSERT_EQ(FirmwareVersion("1.12.45-g123"), FirmwareVersion(1, 12, 45, "g123"));
}

class TestFirmwareVersionExtraVersionInfo : public ::testing::TestWithParam<std::string_view> {};

TEST_P(TestFirmwareVersionExtraVersionInfo, InvalidExtraStringThrows)
{
    EXPECT_THROW(FirmwareVersion(1, 2, 3, GetParam()), std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidExtraString,
    TestFirmwareVersionExtraVersionInfo,
    ::testing::Values("", " ", "hello world", "hello\nworld", "hello\tworld")
);

class TestFirmwareVersionString : public ::testing::TestWithParam<std::string_view> {};

TEST_P(TestFirmwareVersionString, InvalidStringThrows)
{
    EXPECT_THROW(FirmwareVersion{GetParam()}, std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidFirmwareString,
    TestFirmwareVersionString,
    ::testing::Values(
        " ",
        "v1.0.2",
        "v1.0.2-g123",
        "1",
        "1.2",
        "1.2.3.4",
        "1.2.3a",
        "-1.1.1",
        "1.-1.1",
        "1.1.-1",
        "1.1.1-",
        "1.1.1- ",
        "1.1.1-hello world",
        "1.1.1-hello\nworld",
        "1.1.1-hello\tworld"
    )
);

TEST(TestFirmwareVersionString, ConvertsToValidString)
{
    EXPECT_EQ(FirmwareVersion(1, 0, 0).str(), "1.0.0");
    EXPECT_EQ(FirmwareVersion(1, 2, 0).str(), "1.2.0");
    EXPECT_EQ(FirmwareVersion(1, 2, 3).str(), "1.2.3");
    EXPECT_EQ(FirmwareVersion(1, 2, 3, "hello").str(), "1.2.3-hello");
    EXPECT_EQ(FirmwareVersion("1.2.3").str(), "1.2.3");
    EXPECT_EQ(FirmwareVersion("1.2.3-g123").str(), "1.2.3-g123");
}

TEST(TestFirmwareVersionComparison, MajorVersionNumberLessThan)
{
    ASSERT_LT(FirmwareVersion("1.2.3"), FirmwareVersion("2.0.0"));
}

TEST(TestFirmwareVersionComparison, MinorVersionNumberLessThan)
{
    ASSERT_LT(FirmwareVersion("1.2.3"), FirmwareVersion("1.12.0"));
}

TEST(TestFirmwareVersionComparison, PatchVersionNumberLessThan)
{
    ASSERT_LT(FirmwareVersion("1.2.3"), FirmwareVersion("1.2.12"));
}

TEST(TestFirmwareVersionComparison, WithoutExtraVersionInfoLessThanWithVersionInfo)
{
    ASSERT_LT(FirmwareVersion("1.2.3"), FirmwareVersion("1.2.3-g123"));
}

TEST(TestFirmwareVersionComparison, ExtraVersionInfoLessThan)
{
    ASSERT_LT(FirmwareVersion("1.2.3-gabc"), FirmwareVersion("1.2.3-gdef"));
}
