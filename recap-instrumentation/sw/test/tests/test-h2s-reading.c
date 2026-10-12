#include "utest.h"

#include <h2s-reading.h>

#include <limits.h>

#define SERIAL_COMMA H2S_SENSOR_SERIAL_NUMBER ","

#define ASSERT_VALID(str, exp_ppb, exp_temp, exp_humidity)                     \
    do {                                                                       \
        struct h2s_reading reading;                                            \
        ASSERT_TRUE_MSG(                                                       \
            h2s_reading_parse(str, &reading),                                  \
            "Failed to parse H2S string: " str                                 \
        );                                                                     \
        ASSERT_EQ(reading.ppb, exp_ppb);                                       \
        ASSERT_EQ(reading.celsius, exp_temp);                                  \
        ASSERT_EQ(reading.humidity, exp_humidity);                             \
    } while (0)

#define READING_FMT                                                            \
    "(struct h2s_reading) { .ppb = %d, .celsius = %d, .humidity = %d }"
#define READING_FMT_ARGS(r) (r).ppb, (r).celsius, (r).humidity

#define ASSERT_INVALID(str)                                                    \
    do {                                                                       \
        struct h2s_reading reading;                                            \
        if (h2s_reading_parse(str, &reading)) {                                \
            char error_msg[300];                                               \
            const int r = snprintf(                                            \
                error_msg,                                                     \
                sizeof(error_msg),                                             \
                "Parsing should fail for \"%s\", got " READING_FMT,            \
                str,                                                           \
                READING_FMT_ARGS(reading)                                      \
            );                                                                 \
            ASSERT_GT_MSG(r, 0, "unknown snprintf error");                     \
            ASSERT_LT_MSG(                                                     \
                r,                                                             \
                sizeof(error_msg),                                             \
                "snprintf error: buffer not large enough"                      \
            );                                                                 \
            ASSERT_TRUE_MSG(false, error_msg);                                 \
        }                                                                      \
    } while (0)

#define TEST(name) UTEST(h2s_reading, name)

#define TEST_INVALID(name, str)                                                \
    TEST(test_parsing_##name##_fails)                                          \
    {                                                                          \
        ASSERT_INVALID(str);                                                   \
    }

TEST_INVALID(invalid_serial, "INVALIDSERIAL,123,123,50")
TEST_INVALID(serial_missing_comma, H2S_SENSOR_SERIAL_NUMBER "123,123,50")
TEST_INVALID(missing_values, SERIAL_COMMA)
TEST_INVALID(missing_temp_humidity, SERIAL_COMMA "123")
TEST_INVALID(missing_humidity, SERIAL_COMMA "123,123")
TEST_INVALID(empty_string, "")
TEST_INVALID(whitespace, "  ")

TEST_INVALID(ppb_overflow, SERIAL_COMMA "2147483647,0,0");
TEST_INVALID(ppb_underflow, SERIAL_COMMA "-2147483648,0,0");
TEST_INVALID(temp_overflow, SERIAL_COMMA "0,128,0");
TEST_INVALID(temp_underflow, SERIAL_COMMA "0,-129,0");
TEST_INVALID(humidity_overflow, SERIAL_COMMA "0,0,101");
TEST_INVALID(humidity_underflow, SERIAL_COMMA "0,0,-1");

static const char *const invalid_num_strings[] = {
    "0x123",
    "12.2",
    "12.",
    " ",
    "",
    "1 ",
    "abc",
    "123abc",
    "abc123",
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define TEST_INVALID_CHARS(name, fmt)                                          \
    TEST(test_parsing_##name##_fails)                                          \
    {                                                                          \
        for (size_t i = 0; i < ARRAY_SIZE(invalid_num_strings); i++) {         \
            char str_to_parse[200];                                            \
            const int r = snprintf(                                            \
                str_to_parse,                                                  \
                sizeof(str_to_parse),                                          \
                SERIAL_COMMA fmt,                                              \
                invalid_num_strings[i]                                         \
            );                                                                 \
            ASSERT_LT_MSG(                                                     \
                r,                                                             \
                sizeof(str_to_parse),                                          \
                "snprintf error: buffer not large enough"                      \
            );                                                                 \
            ASSERT_GT_MSG(r, 0, "unknown snprintf error");                     \
            ASSERT_INVALID(str_to_parse);                                      \
        }                                                                      \
    }

TEST_INVALID_CHARS(invalid_ppb_str, "%s,0,0")
TEST_INVALID_CHARS(invalid_temp_str, "0,%s,0")
TEST_INVALID_CHARS(invalid_humidity_str, "0,0,%s")

#define MAX_TEMP_STR "127"
#define MAX_TEMP INT8_MAX
#define MIN_TEMP_STR "-128"
#define MIN_TEMP INT8_MIN

#define TEST_VALID(name, str, ppb, temp, humidity)                             \
    TEST(test_parses_##name)                                                   \
    {                                                                          \
        ASSERT_VALID(SERIAL_COMMA str, ppb, temp, humidity);                   \
    }

TEST_VALID(zeros, "0,0,0", 0, 0, 0)
TEST_VALID(positive, "321,123, 50", 321, 123, 50)
TEST_VALID(negative, "-321,-123,50", -321, -123, 50)
TEST_VALID(leading_zeros_are_decimal, "010,011,012", 10, 11, 12)

TEST_VALID(max_ppb, "2147483646,0,0", INT32_MAX - 1, 0, 0)
TEST_VALID(min_ppb, "-2147483647,0,0", INT32_MIN + 1, 0, 0)

TEST_VALID(max_temp, "0,127,0", 0, INT8_MAX, 0)
TEST_VALID(min_temp, "0,-128,0", 0, INT8_MIN, 0)

TEST_VALID(max_humidity, "0,0,100", 0, 0, 100)
TEST_VALID(min_humidity, "0,0,0", 0, 0, 0)

TEST(test_parses_with_spaces)
{
    ASSERT_VALID(SERIAL_COMMA " 321,-123, 50", 321, -123, 50);
    ASSERT_VALID(SERIAL_COMMA " -321, 123, 50", -321, 123, 50);
}

UTEST_MAIN()
