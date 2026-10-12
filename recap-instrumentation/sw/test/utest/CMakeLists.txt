set(UTEST_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)

function(add_utest_test TARGET)
    # The following is based on GTest's CMake integration. We add a post-build
    # command on the test runner executable that runs the executable to get a
    # list of test names, and then generate CTest tests from these names.

    add_executable(${TARGET} ${ARGN})
    target_include_directories(${TARGET} PRIVATE ${UTEST_DIR})

    set(test_base "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}")
    set(tests_file "${test_base}_tests.cmake")
    set(include_file "${test_base}_include.cmake")
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        BYPRODUCTS ${tests_file}
        COMMAND
            ${CMAKE_COMMAND}
                -DTEST_RUNNER=$<TARGET_FILE:${TARGET}>
                -DTEST_GROUP=${TARGET}
                -DTEST_FILE=${tests_file}
                -P ${UTEST_DIR}/add_utest_tests.cmake
    )

    # When ctest runs, it will include ${include_file}, which itself includes
    # ${tests_file} that adds the tests (add_test cannot be called directly
    # inside script mode).
    file(WRITE "${include_file}"
        "if(EXISTS \"${tests_file}\")\n"
        "  include(\"${tests_file}\")\n"
        "else()\n"
        "  add_test(${TARGET}_NOT_BUILT ${TARGET}_NOT_BUILT)\n"
        "endif()\n"
    )
    set_property(DIRECTORY
      APPEND PROPERTY TEST_INCLUDE_FILES "${include_file}"
    )
endfunction()
