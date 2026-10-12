find_program(UV uv REQUIRED)
set(UV_RUN ${UV} run --group lint)

execute_process(
    COMMAND ${UV_RUN} clang-format --version
    OUTPUT_VARIABLE CLANG_FORMAT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
message(STATUS "Using clang-format: ${CLANG_FORMAT_VERSION}")

execute_process(
    COMMAND ${UV_RUN} cppcheck --version
    OUTPUT_VARIABLE CPPCHECK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
message(STATUS "Using cppcheck: ${CPPCHECK_VERSION}")

add_custom_target(
    check
    COMMAND ${UV_RUN} cppcheck
        --force
        --enable=style
        --std=c++20
        --error-exitcode=1
        --inline-suppr
        --quiet
        ${CMAKE_SOURCE_DIR}/cli
        ${CMAKE_SOURCE_DIR}/gui
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/test
        ${CMAKE_SOURCE_DIR}/util
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking source code with cppcheck"
    USES_TERMINAL
)

set(VENV_DIR ${CMAKE_SOURCE_DIR}/.venv)
if(WIN32)
    set(CLANG_FORMAT_EXE ${VENV_DIR}/Scripts/clang-format.exe)
else()
    set(CLANG_FORMAT_EXE ${VENV_DIR}/bin/clang-format)
endif()

set(
    CLANG_FORMAT_CMD
    ${UV_RUN} ${CMAKE_SOURCE_DIR}/scripts/run-clang-format.py
    --clang-format-executable ${CLANG_FORMAT_EXE}
    --exclude=${CMAKE_BINARY_DIR}
    --recursive
    ${CMAKE_SOURCE_DIR}
)

add_custom_target(
    lint
    COMMAND ${CLANG_FORMAT_CMD}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Linting source code with clang-format"
    USES_TERMINAL
)
add_custom_target(lint-all DEPENDS check lint)

add_custom_target(
    format
    COMMAND ${CLANG_FORMAT_CMD} --in-place
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting source code with clang-format"
    USES_TERMINAL
)
