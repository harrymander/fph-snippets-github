cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

include(FetchContent)

if(WIN32)
    # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
fetchcontent_declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG d72f9c8aea6817cdf1ca0ac10887f328de7f3da2
)
fetchcontent_makeavailable(googletest)

include(GoogleTest)

macro(add_lib_test TARGET FILES)
    add_executable(${TARGET} ${FILES})
    target_link_libraries(${TARGET} PRIVATE GTest::gtest_main usb-reader)
    gtest_discover_tests(${TARGET})
endmacro()

add_lib_test(test-firmware-version test-firmware-version.cpp)
add_lib_test(test-record-header test-record-header.cpp)
add_lib_test(test-record-sample test-record-sample.cpp)

find_program(UV uv REQUIRED)
add_executable(test-record-writer test-record-writer.cpp)
target_link_libraries(test-record-writer PRIVATE usb-reader)
add_test(
    NAME test-record-writer
    COMMAND
        ${UV} run
        ${CMAKE_CURRENT_SOURCE_DIR}/test-record-writer.py
        $<TARGET_FILE:test-record-writer>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

message(CHECK_START "Finding valgrind")
find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM STREQUAL VALGRIND_PROGRAM-NOTFOUND)
    message(CHECK_FAIL
        "Could not find valgrind - tests will not run with memory checking")
else()
    message(CHECK_PASS "Found valgrind")
    set(VALGRIND_PROGRAM
        ${VALGRIND_PROGRAM} --leak-check=full --error-exitcode=1)
endif()

macro(add_valgrind_test TARGET)
    if(VALGRIND_PROGRAM STREQUAL VALGRIND_PROGRAM-NOTFOUND)
        add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:${TARGET}>)
    else()
        add_test(
            NAME ${TARGET}
            COMMAND ${VALGRIND_PROGRAM}
                --leak-check=full
                --error-exitcode=1
                --quiet
                -- $<TARGET_FILE:${TARGET}>
        )
    endif()
endmacro()

add_executable(test-observer test-observer.cpp)
target_include_directories(test-observer PRIVATE ${CMAKE_SOURCE_DIR}/gui/src)
target_link_libraries(test-observer PRIVATE GTest::gtest_main)
target_compile_features(test-observer PUBLIC cxx_std_20)
add_valgrind_test(test-observer)
