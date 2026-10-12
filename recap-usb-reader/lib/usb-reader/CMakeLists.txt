include(FetchContent)

fetchcontent_declare(
    serial
    GIT_REPOSITORY https://github.com/harrymander/serial.git
    GIT_TAG baecc995857690e91c667156f763c2e817f334b8
)
fetchcontent_makeavailable(serial)

set(srcdir ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(
    usb-reader STATIC
    ${srcdir}/header.cpp
    ${srcdir}/record-stream.cpp
    ${srcdir}/sample.cpp
    ${srcdir}/version.cpp
    ${srcdir}/writer.cpp
)
target_compile_features(usb-reader PUBLIC cxx_std_20)
target_link_libraries(usb-reader PUBLIC serial)

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(
    usb-reader
    PRIVATE
        $<BUILD_INTERFACE:${include_dir}/recap/usb-reader>
    PUBLIC
        $<BUILD_INTERFACE:${include_dir}>
)

if(WIN32)
    target_compile_options(usb-reader PRIVATE /W4)
else()
    target_compile_options(
        usb-reader
        PRIVATE -Wall -Wextra $<$<CONFIG:DEBUG>:-Og>
    )
endif()

option(
    RECAP_USB_READER_MOCK_READER
    "Mock the reader object to generate random sensor data"
    OFF
)
if(RECAP_USB_READER_MOCK_READER)
    message(STATUS "Using mocked reader object")
    target_sources(usb-reader PRIVATE ${srcdir}/reader-mock-impl.cpp)
    if(WIN32)
        target_link_libraries(usb-reader PRIVATE Winmm.lib)
    endif()
else()
    message(STATUS "Not using mocked reader object")
    target_sources(usb-reader PRIVATE ${srcdir}/reader-serial-impl.cpp)
endif()
