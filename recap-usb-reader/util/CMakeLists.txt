add_executable(xxd ${CMAKE_CURRENT_SOURCE_DIR}/xxd.c)

macro(generate_c_array_from_file)
    set(options INPUT OUTPUT ARRAY_NAME)
    cmake_parse_arguments(ARG "" "${options}" "" ${ARGN})
    if(NOT DEFINED ARG_OUTPUT)
        message(SEND_ERROR "generate_c_array_from_file: OUTPUT is required")
    endif()
    if(NOT DEFINED ARG_INPUT)
        message(SEND_ERROR "generate_c_array_from_file: INPUT is required")
    endif()
    if(NOT DEFINED ARG_ARRAY_NAME)
        set(ARG_ARRAY_NAME "data")
    endif()
    get_filename_component(dirname "${ARG_OUTPUT}" DIRECTORY)
    add_custom_command(
        OUTPUT ${ARG_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dirname}"
        COMMAND xxd "${ARG_INPUT}" "${ARG_ARRAY_NAME}" > "${ARG_OUTPUT}"
        MAIN_DEPENDENCY ${ARG_INPUT}
        DEPENDS xxd ${ARG_INPUT}
    )
endmacro()
