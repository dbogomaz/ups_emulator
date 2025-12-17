# @file tests/cmake/add_ups_test.cmake
# ===== Функция добавления тестов =====
function(add_ups_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})

    target_link_libraries(${TEST_NAME} PRIVATE
        ups_core
        GTest::gtest_main
    )

    set_target_properties(${TEST_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
    )

    target_compile_options(${TEST_NAME} PRIVATE ${WARN_FLAGS})

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    # Включить Coverage для тестов
    if (BUILD_COVERAGE)
        target_compile_options(${TEST_NAME} PRIVATE ${COVERAGE_COMPILE_FLAGS})
        target_link_options(${TEST_NAME} PRIVATE ${COVERAGE_LINK_FLAGS})
    endif()
endfunction()
