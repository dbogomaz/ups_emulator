# @file tests/CMakeLists.txt
# ============================================================
#                         BUILD TESTS
# ============================================================

message(STATUS "Configuring BUILD TESTS subsystem")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_ups_test.cmake)

# ===== Добавление тестов =====
add_ups_test(ups_model_config_test
    unit/ups_model_config_test.cpp
)

add_subdirectory(googletest)

if (BUILD_COVERAGE)
    add_subdirectory(coverage)
endif()
