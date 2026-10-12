cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(test-recap-instrumentation VERSION 0.1.0 LANGUAGES C)

add_compile_options(-Wall -std=gnu11 -g)

enable_testing()
add_subdirectory(utest)

add_utest_test(
    test-h2s-reading
    tests/test-h2s-reading.c
    ../drivers/h2s/h2s-reading.c)
target_compile_definitions(
    test-h2s-reading PUBLIC
    H2S_SENSOR_SERIAL_NUMBER="SERIALNUM"
)
target_include_directories(test-h2s-reading PUBLIC ../drivers/h2s tests)
