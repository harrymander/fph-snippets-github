fetchcontent_declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG af442b4da0cd7a07b56fa709bd16571889dc7fda
)
fetchcontent_makeavailable(argparse)

add_executable(recapctl ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_compile_features(recapctl PRIVATE cxx_std_20)
target_link_libraries(
    recapctl
    PRIVATE argparse::argparse usb-reader utils)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(BUILD_TYPE "$<$<CONFIG:DEBUG>:un->optimised")
else()
    set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()
set(BUILD_INFO "recapctl v${PROJECT_VERSION} \
(${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}, ${BUILD_TYPE} build)")

target_compile_definitions(
    recapctl
    PRIVATE
        PROJECT_NAME="recapctl"
        BUILD_INFO="${BUILD_INFO}"
)

if(UNIX)
    target_compile_definitions(recapctl PRIVATE __unix__)
    target_compile_options(recapctl
        PRIVATE -Wall -Wextra $<$<CONFIG:DEBUG>:-Og>
    )
elseif(WIN32)
    target_link_libraries(recapctl PRIVATE Kernel32.lib)
    target_compile_options(recapctl PRIVATE /W4)
else()
    message(SEND_ERROR "Unsupported platform")
endif()

install(TARGETS recapctl DESTINATION bin)
