set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

find_program(Inkscape inkscape HINTS ${INKSCAPE_HINTS_DIR} REQUIRED)
set(ICON_SVG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/icon.svg)
set(ICON_PNG_PATH ${GEN_DIR}/icon.png)
if(WIN32)
    set(ICON_PNG_DIM 256)
else()
    set(ICON_PNG_DIM 512)
endif()

# First convert the SVG file into a PNG file with Inkscape
add_custom_command(
    OUTPUT ${ICON_PNG_PATH}
    COMMAND
        ${Inkscape} ${ICON_SVG_PATH} -o ${ICON_PNG_PATH}
        --export-area-page -w ${ICON_PNG_DIM} -h ${ICON_PNG_DIM}
    DEPENDS ${ICON_SVG_PATH}
    COMMENT "Generating icon .png file from .svg"
)

if(WIN32)
    # On Windows, convert the PNG file into an ICO file and link it into the
    # .exe as a resource.
    find_program(ImageMagick NAMES magick convert REQUIRED)

    set(ICON_ICO_PATH ${GEN_DIR}/icon.ico)
    add_custom_command(
        OUTPUT ${ICON_ICO_PATH}
        COMMAND
            ${ImageMagick}
                -background transparent
                ${ICON_PNG_PATH}
                -define icon ${ICON_ICO_PATH}
        DEPENDS ${ICON_PNG_PATH}
        COMMENT "Generating icon .ico file from .png"
    )

    set(ICON_RC_PATH ${GEN_DIR}/icon.rc)
    file(WRITE ${ICON_RC_PATH} "APPICON ICON \"icon.ico\"")
    set_property(
        SOURCE ${ICON_RC_PATH}
        APPEND PROPERTY OBJECT_DEPENDS ${ICON_ICO_PATH}
    )
    add_library(app_icon OBJECT ${ICON_RC_PATH})
    set_target_properties(
        app_icon
        PROPERTIES ICO_FILE ${ICON_ICO_PATH}
    )
else()
    # Convert the PNG to a GLFW-compatible C array using Python
    find_program(UV uv REQUIRED)
    set(icon_c_path ${GEN_DIR}/glfw-icon.c)
    add_custom_command(
        OUTPUT ${icon_c_path}
        COMMAND
            ${UV} run --group build
                "${CMAKE_CURRENT_SOURCE_DIR}/generate-glfw-image.py"
                --varname recap_gui_glfw_icon_img
                "${ICON_PNG_PATH}"
                "${icon_c_path}"
        DEPENDS
            ${ICON_PNG_PATH}
            ${CMAKE_CURRENT_SOURCE_DIR}/generate-glfw-image.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_library(app_icon OBJECT ${icon_c_path})

    if(LINUX)
        install(
            FILES ${ICON_SVG_PATH}
            DESTINATION share/icons/hicolor/scalable/apps
            RENAME recapctl-gui.svg
        )
        install(
            FILES ${ICON_PNG_PATH}
            DESTINATION share/icons/hicolor/512x512/apps
            RENAME recapctl-gui.png
        )
    endif()
endif()
