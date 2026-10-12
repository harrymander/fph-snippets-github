if(NOT TEST_RUNNER)
    message(FATAL_ERROR "TEST_RUNNER not defined")
endif()
if(NOT TEST_GROUP)
    message(FATAL_ERROR "TEST_GROUP not defined")
endif()
if(NOT TEST_FILE)
    message(FATAL_ERROR "TEST_FILE not defined")
endif()

# Running the test runner with just the --list-tests flag prints a list of all
# the tests' names. We loop over this list and create a separate test for each
# of them.
execute_process(
    COMMAND ${TEST_RUNNER} --list-tests
    OUTPUT_VARIABLE TEST_NAMES
    COMMAND_ERROR_IS_FATAL ANY
)

set(TEST_FILE_CONTENT "")
string(REGEX REPLACE "\n" ";" TEST_NAMES "${TEST_NAMES}")
foreach(TEST_NAME ${TEST_NAMES})
    # Note add_test when run inside ctest has a different signature to add_test
    # when run from cmake
    # (https://cmake.org/cmake/help/latest/command/add_test.html). The ctest
    # version doesn't require the NAME and COMMAND keywords.
    string(
        APPEND TEST_FILE_CONTENT
        "add_test(\n"
        "  [=[${TEST_GROUP}:${TEST_NAME}]=]\n"
        "  [=[${TEST_RUNNER}]=] [=[--filter=${TEST_NAME}]=] --filter-strict\n"
        ")\n"
    )
endforeach()
file(WRITE ${TEST_FILE} "${TEST_FILE_CONTENT}")
