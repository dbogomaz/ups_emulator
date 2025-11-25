# @file tests/CMakeLists.txt
# ============================================================
#                         BUILD TESTS
# ============================================================

message(STATUS "Configuring BUILD TESTS subsystem")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_ups_test.cmake)

# === Папка для тестовых данных ===
set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")
add_compile_definitions(TEST_DATA_DIR=\"${TEST_DATA_DIR}\")

# ===== Добавление тестов =====
add_ups_test(ups_model_config_test
    unit/ups_model_config_test.cpp
)

add_ups_test(fs_utils_test
    unit/fs_utils_test.cpp
)

add_subdirectory(googletest)

if (BUILD_COVERAGE)
    add_subdirectory(coverage)
endif()
