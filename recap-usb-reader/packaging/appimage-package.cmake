set(APPIMAGETOOL ${CPACK_RECAP_USB_READER_BINARY_DIR}/appimagetool.appimage)

set(
    APPIMAGETOOL_URL_BASE
    https://github.com/AppImage/appimagetool/releases/download/continuous
)
file(
    DOWNLOAD
    "${APPIMAGETOOL_URL_BASE}/appimagetool-x86_64.AppImage"
    ${APPIMAGETOOL}
    EXPECTED_HASH MD5=7c8158dd43bf24b1a30b8774f71a8d38
    STATUS DOWNLOAD_STATUS
)
list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)
list(GET DOWNLOAD_STATUS 1 DOWNLOAD_STATUS_MSG)
if(DOWNLOAD_STATUS_CODE EQUAL 0)
    message(STATUS "Downloaded appimagetool")
else()
    message(FATAL_ERROR
        "Error downloading appimagetool: ${DOWNLOAD_STATUS_MSG}"
    )
endif()

file(
    CHMOD ${APPIMAGETOOL}
    FILE_PERMISSIONS
        OWNER_EXECUTE OWNER_READ OWNER_WRITE
        GROUP_EXECUTE GROUP_READ GROUP_WRITE
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
            APPIMAGE_STAGING=${CPACK_TEMPORARY_DIRECTORY}
            APPIMAGETOOL=${APPIMAGETOOL}
            APPIMAGE_OUTPUT=${CPACK_PACKAGE_FILE_NAME}.appimage
            ${CPACK_RECAP_USB_READER_SOURCE_DIR}/packaging/make-appimage.sh
    COMMAND_ERROR_IS_FATAL ANY
)
