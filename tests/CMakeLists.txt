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
    unit/ups_model_config/test_load_errors.cpp
    unit/ups_model_config/test_load_success.cpp
    unit/ups_model_config/test_valueset_errors.cpp
)

add_ups_test(fs_utils_test
    unit/fs_utils_test.cpp
)

add_ups_test(ups_data_store_test
    unit/ups_data_store_test.cpp
)

add_ups_test(snmp_codec_test
    unit/snmp_codec/test_decode_basic.cpp
    unit/snmp_codec/test_decode_errors.cpp
)

add_subdirectory(googletest)

if (BUILD_COVERAGE)
    add_subdirectory(coverage)
endif()
