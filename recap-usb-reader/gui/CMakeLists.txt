add_subdirectory(icon)
add_subdirectory(vendor)

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
set(GEN_INCLUDE_DIR ${GEN_DIR}/include)

set(srcdir ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(
    recapctl-gui
    ${srcdir}/app.cpp
    ${srcdir}/device.cpp
    ${srcdir}/filename.cpp
    ${srcdir}/writer-observer.cpp
    ${srcdir}/gui/controller.cpp
    ${srcdir}/gui/directory.cpp
    ${srcdir}/gui/font.cpp
    ${srcdir}/gui/gui.cpp
    ${srcdir}/gui/misc.cpp
    ${srcdir}/gui/plots.cpp
    ${srcdir}/gui/strhelpers.cpp
)
target_compile_features(recapctl-gui PUBLIC cxx_std_20)
target_include_directories(
    recapctl-gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GEN_INCLUDE_DIR}
    ${srcdir}
)
target_link_libraries(recapctl-gui PUBLIC
    app_icon
    imgui::imgui
    implot::implot
    magic_enum::magic_enum
    nfd::nfd
    usb-reader
    utils
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(BUILD_TYPE "$<$<CONFIG:DEBUG>:un->optimised")
else()
    set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()
set(BUILD_INFO "recapctl-gui v${PROJECT_VERSION} \
(${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}, ${BUILD_TYPE} build)")

target_compile_definitions(
    recapctl-gui PUBLIC
    PROGRAM_NAME="recapctl-gui"
    VERSION_STR="${PROJECT_VERSION}"
    BUILD_INFO="${BUILD_INFO}"
)

option(RECAP_USB_READER_GUI_DISABLE_DEMOS "Disable ImGui/ImPlot demos" OFF)
if(RECAP_USB_READER_GUI_DISABLE_DEMOS)
    message(STATUS "ImGui/ImPlot demos disabled")
    set(RECAP_USB_READER_GUI_INCLUDE_DEMOS OFF)
else()
    message(STATUS "ImGui/ImPlot demos enabled")
    target_link_libraries(recapctl-gui PUBLIC imgui::demo implot::demo)
    set(RECAP_USB_READER_GUI_INCLUDE_DEMOS ON)
endif()
configure_file(config/app.h.in ${GEN_INCLUDE_DIR}/config/app.h)

add_subdirectory(platform)
include(fonts.cmake)

set(LICENSE_INFO
    "Dear ImGui" ${IMGUI_LICENSE}
    "Font Awesome" ${fontawesome_SOURCE_DIR}/LICENSE.txt
    "ImPlot" ${IMPLOT_LICENSE}
    "IconFontCppHeaders" ${iconfontcppheaders_SOURCE_DIR}/licence.txt
    "magic_enum" ${magic_enum_SOURCE_DIR}/LICENSE
    "Native File Dialog Extended" ${NFD_LICENSE}
    "Serial" ${serial_SOURCE_DIR}/LICENSE
)
if(RECAP_USB_READER_GUI_EMBED_FONT)
    list(APPEND LICENSE_INFO "Adobe Source Sans" ${SOURCE_SANS_LICENSE})
endif()
set(
    GEN_LICENSE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate-license-header.py
)
set(LICENSE_INFO_HEADER ${GEN_INCLUDE_DIR}/license-info.hpp)
find_program(UV uv REQUIRED)
add_custom_command(
    OUTPUT ${LICENSE_INFO_HEADER}
    DEPENDS ${GEN_LICENSE_HEADER}
    COMMAND
        ${UV} run ${GEN_LICENSE_HEADER}
        ${LICENSE_INFO}
        --output=${LICENSE_INFO_HEADER}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating license header"
)
add_custom_target(
    license-info-header
    DEPENDS ${LICENSE_INFO_HEADER}
)
add_dependencies(recapctl-gui license-info-header)

add_custom_target(run-gui DEPENDS recapctl-gui COMMAND recapctl-gui)

install(TARGETS recapctl-gui DESTINATION bin)
if(LINUX)
    configure_file(
        recapctl-gui.desktop.template
        ${CMAKE_CURRENT_BINARY_DIR}/gen/recapctl-gui.desktop
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gen/recapctl-gui.desktop
        DESTINATION share/applications
    )
endif()
