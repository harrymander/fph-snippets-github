# Variables with CPACK_ are propagated to cpack scripts
set(CPACK_RECAP_USB_READER_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(CPACK_RECAP_USB_READER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Harry Mander <harry.mander@pg.canterbury.ac.nz>")
if(LINUX)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0, libglfw3")
    set(CPACK_EXTERNAL_ENABLE_STAGING ON)
    set(
        CPACK_EXTERNAL_PACKAGE_SCRIPT
        ${CMAKE_CURRENT_SOURCE_DIR}/appimage-package.cmake
    )
elseif(WIN32)
    # Installed package name (in start menu/desktop shortcuts etc.):
    set(CPACK_NSIS_PACKAGE_NAME "RECAP USB Reader")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_NSIS_PACKAGE_NAME}")
    set(CPACK_PACKAGE_VENDOR "Harry Mander")
    set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")

    # Name in "Remove programs" list
    set(CPACK_NSIS_DISPLAY_NAME
        "${CPACK_NSIS_PACKAGE_NAME} ${PROJECT_VERSION}"
    )

    # Executable that contains the icon:
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\recapctl-gui.exe")

    set(CPACK_CREATE_DESKTOP_LINKS recapctl-gui)
    set(CPACK_NSIS_MENU_LINKS
        bin/recapctl-gui.exe "${CPACK_NSIS_PACKAGE_NAME}"
    )
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_IGNORE_LICENSE_PAGE ON)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN recapctl-gui)
    get_target_property(CPACK_NSIS_MUI_ICON app_icon ICO_FILE)
    get_target_property(CPACK_NSIS_MUI_UNIICON app_icon ICO_FILE)
endif()

include(CPack)
