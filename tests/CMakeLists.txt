# @file tests/CMakeLists.txt
# ============================================================
#                         BUILD TESTS
# ============================================================
message(STATUS "Configuring BUILD TESTS subsystem")

# ===== GTest =====
add_subdirectory(${CMAKE_SOURCE_DIR}/tests/googletest)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_ups_test.cmake)

# ===== Добавление тестов =====
add_ups_test(ups_model_config_test
    unit/ups_model_config_test.cpp
)

# ============================================================
#               ACTIVATE COVERAGE SUBSYSTEM
# ============================================================
if (BUILD_COVERAGE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/coverage)
endif()
