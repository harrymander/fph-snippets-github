include(FetchContent)

option(
    RECAP_USB_READER_GUI_EMBED_FONT
    "Embed Adobe Source Sans font, rather than a system font"
    OFF
)
if(RECAP_USB_READER_GUI_EMBED_FONT)
    include(source_sans.cmake)
    target_sources(recapctl-gui PUBLIC ${SOURCE_SANS_C_PATH})
    set(RECAP_USB_READER_GUI_FONT_SIZE 20 CACHE STRING "Default font size")
    message(
        STATUS
        "Using embedded font default size ${RECAP_USB_READER_GUI_FONT_SIZE}"
    )
else()
    set(default_font_path OFF)
    set(defaut_font_size 0)
    if(WIN32)
        set(default_font_path "C:\\Windows\\Fonts\\segoeui.ttf")
        set(default_font_size 18)
    elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
        # Will only work on Ubuntu
        set(default_font_path "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf")
        set(default_font_size 16)
    endif()

    set(
        RECAP_USB_READER_GUI_FONT_PATH ${default_font_path}
        CACHE STRING "Default font path"
    )
    set(
        RECAP_USB_READER_GUI_FONT_SIZE ${default_font_size}
        CACHE STRING "Default font size"
    )
    if(RECAP_USB_READER_GUI_FONT_PATH OR RECAP_USB_READER_GUI_FONT_SIZE)
        if(NOT(
            RECAP_USB_READER_GUI_FONT_PATH
            AND RECAP_USB_READER_GUI_FONT_SIZE
        ))
            message(SEND_ERROR "Must specify both font path and size")
        endif()
        message(
            STATUS
            "Using system font ${RECAP_USB_READER_GUI_FONT_PATH} "
            "default size ${RECAP_USB_READER_GUI_FONT_SIZE}"
        )
    else()
        message(STATUS "Using built-in Dear Imgui font")
    endif()
endif()
configure_file(config/font.h.in ${GEN_INCLUDE_DIR}/config/font.h)

include(fontawesome.cmake)
target_sources(recapctl-gui PUBLIC ${FA_FONT_C_PATH})

fetchcontent_declare(
    iconfontcppheaders
    GIT_REPOSITORY https://github.com/juliettef/IconFontCppHeaders.git
    GIT_TAG 1a083cca7d650c0615e32a1d39d56892a2ce8c5b
)
fetchcontent_makeavailable(iconfontcppheaders)
target_include_directories(
    recapctl-gui
    PRIVATE ${iconfontcppheaders_SOURCE_DIR}
)
