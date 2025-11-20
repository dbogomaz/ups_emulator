cmake_minimum_required(VERSION 3.10.0)

project(ups_emulator)

set(PROJECT_SRC
    src/main.cpp
    src/ups_model_config.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Создаём папку config в build/ если её нет и копируем туда ini-файл
set(CONFIG_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/config)
file(MAKE_DIRECTORY ${CONFIG_FOLDER})
file(COPY ups_models.ini DESTINATION ${CONFIG_FOLDER})


set(TESTS_SRC
    tests/ups_model_config_test.cpp
)

set(TEST_INCLUDE
    src/
)

add_executable(ups_model_config_test ${TESTS_SRC})
target_include_directories(ups_model_config_test PRIVATE ${TEST_INCLUDE})
target_link_libraries(ups_model_config_test GTest::gtest_main)
set_target_properties(ups_model_config_test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# ********************** Подключение GoogleTest *************
add_subdirectory(${CMAKE_SOURCE_DIR}/tests/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
target_link_libraries(${PROJECT_NAME} GTest::gtest_main)

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
message(STATUS "GTest added for target: ${PROJECT_NAME}")
