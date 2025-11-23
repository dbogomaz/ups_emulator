cmake_minimum_required(VERSION 3.10.0)

# ============================================================
#                     PROJECT CONFIG
# ============================================================
project(ups_emulator)

set(PROJECT_INCLUDE_DIR 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
)

set(WARN_FLAGS
    -Wall
    -Wextra
    -Wpedantic
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# ============================================================
#               OPTIONS (TESTING, COVERAGE)
# ============================================================
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_COVERAGE "Enable coverage instrumentation" ON)
set(COVERAGE_COMPILE_FLAGS --coverage -O0 -g)
set(COVERAGE_LINK_FLAGS --coverage)


# ============================================================
#                   UPS CORE LIBRARY
# ============================================================
set(UPS_CORE_SRC
    src/ups_model_config.cpp

    src/utils/fs_utils.cpp
    src/utils/string_utils.cpp
)

add_library(ups_core ${UPS_CORE_SRC})

target_include_directories(ups_core PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

set_target_properties(ups_core PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(ups_core PRIVATE ${WARN_FLAGS})

if (BUILD_COVERAGE)
    target_compile_options(ups_core PRIVATE ${COVERAGE_COMPILE_FLAGS})
endif()



# ============================================================
#                   MAIN APPLICATION
# ============================================================
add_executable(ups_emulator
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ups_core)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(${PROJECT_NAME} PRIVATE ${WARN_FLAGS})

if (BUILD_COVERAGE)
    target_link_options(${PROJECT_NAME} PRIVATE ${COVERAGE_LINK_FLAGS})
endif()



# ============================================================
#                     BUILD TESTS (optional)
# ============================================================
if (BUILD_TESTING)

    enable_testing()

    # ===== GTest =====
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests/googletest)

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
        )

        target_compile_options(${TEST_NAME} PRIVATE ${WARN_FLAGS})

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

        # Включить Coverage для тестов
        if (BUILD_COVERAGE)
            target_compile_options(${TEST_NAME} PRIVATE ${COVERAGE_COMPILE_FLAGS})
            target_link_options(${TEST_NAME} PRIVATE ${COVERAGE_LINK_FLAGS})
        endif()
    endfunction()

    # ===== Добавление тестов =====
    add_ups_test(ups_model_config_test
        tests/unit/ups_model_config_test.cpp
    )

endif()



# ============================================================
#               ACTIVATE COVERAGE SUBSYSTEM
# ============================================================
if (BUILD_COVERAGE)
    add_subdirectory(tests/coverage)
endif()



# ============================================================
#                COPY CONFIG FILES TO BUILD/
# ============================================================
set(CONFIG_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config)
file(MAKE_DIRECTORY ${CONFIG_FOLDER})
file(COPY ${CMAKE_SOURCE_DIR}/ups_models.ini DESTINATION ${CONFIG_FOLDER})
