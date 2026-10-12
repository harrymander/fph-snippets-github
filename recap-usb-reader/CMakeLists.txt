cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

if(APPLE)
    set(LANGUAGES C CXX OBJCXX)
else()
    set(LANGUAGES C CXX)
endif()

# When downloading via URLs with fetchcontent, and
# DOWNLOAD_EXTRACT_TIMESTAMP is not set, set the timestamp of the extracted
# contents to the time of extraction.
cmake_policy(SET CMP0135 NEW)

execute_process(
    COMMAND git describe --tags --match=v* --abbrev=0
    OUTPUT_VARIABLE GIT_VERSION_TAG
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
string(SUBSTRING "${GIT_VERSION_TAG}" 1 -1 GIT_VERSION_TAG)

project(
    recap-usb-reader
    VERSION ${GIT_VERSION_TAG}
    LANGUAGES ${LANGUAGES}
    DESCRIPTION "RECAP data collection device USB reader"
)

if(WIN32)
    # Disables the min/max preprocessor macros, so not to interfere with
    # min/max in the STL
    add_compile_definitions(NOMINMAX)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    option(
        GLIBCXX_DEBUG_MODE
        "Enable libstdc++ debug mode for Debug builds"
        ON
    )
    if(GLIBCXX_DEBUG_MODE)
        add_compile_definitions($<$<CONFIG:Debug>:_GLIBCXX_DEBUG=1>)

        # Enable debug backtrace - only supported on GCC 13+(?). find_library
        # doesn't seem to work with system libraries, so try to compile a C++
        # source file with library linked instead to determine if backtrace is
        # supported
        include(CheckCXXSourceCompiles)
        set(backtrace_lib stdc++_libbacktrace)
        set(CMAKE_REQUIRED_LIBRARIES ${backtrace_lib})
        check_cxx_source_compiles("int main() {}" HAS_BACKTRACE)
        if(HAS_BACKTRACE)
            add_compile_definitions(
                $<$<CONFIG:Debug>:_GLIBCXX_DEBUG_BACKTRACE=1>
            )
            link_libraries(debug ${backtrace_lib})
        endif()
    endif()
endif()

add_subdirectory(lib)

# If building this project directly rather than via fetchcontent
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(util)

    option(RECAP_USB_READER_BUILD_CLI "Build recapctl CLI" ON)
    if(RECAP_USB_READER_BUILD_CLI)
        add_subdirectory(cli)
    endif()

    option(RECAP_USB_READER_BUILD_GUI "Build recapctl GUI" ON)
    if(RECAP_USB_READER_BUILD_GUI)
        add_subdirectory(gui)
    endif()

    option(RECAP_USB_READER_BUILD_TESTS "Build unit tests" ON)
    if(RECAP_USB_READER_BUILD_TESTS)
        enable_testing()
        add_subdirectory(test)
    endif()

    option(RECAP_USB_READER_ADD_LINT_TARGETS "Add targets for linting" ON)
    if(RECAP_USB_READER_ADD_LINT_TARGETS)
        include(cmake/lint.cmake)
    endif()

    add_subdirectory(packaging)
endif()
