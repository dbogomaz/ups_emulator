cmake_minimum_required(VERSION 3.10.0)

project(ups_emulator)

set(PROJECT_SRC
    src/main.cpp
    src/ups_model_config.cpp
)

set(PROJECT_INCLUDES
   ${CMAKE_SOURCE_DIR}/src
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
    src/ups_model_config.cpp
)

set(TEST_FILE ups_model_config_test)

add_executable(${TEST_FILE} ${TESTS_SRC})
target_include_directories(${TEST_FILE} PRIVATE ${PROJECT_INCLUDES})
target_link_libraries(${TEST_FILE} GTest::gtest_main)
set_target_properties(${TEST_FILE} PROPERTIES
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

option(BUILD_COVERAGE "Whether to build coverage or not" ON)
if (BUILD_COVERAGE)
    # get_filename_component(PROJECT_SRC_ABS "${PROJECT_SRC}" REALPATH)

    # === Файлы покрытия ===
    set(COVERAGE_FILES
        "${CMAKE_SOURCE_DIR}/src/ups_model_config.cpp"
    )

    target_include_directories(${TEST_FILE} PRIVATE ${TEST_INCLUDE})

    target_compile_options(${TEST_FILE} PRIVATE --coverage)
    target_link_options(${TEST_FILE} PRIVATE --coverage)

    # === Пути ===
    set(COVERAGE_LIST_FILE   "${CMAKE_BINARY_DIR}/coverage_file_list.txt")
    set(COVERAGE_SCRIPT_NAME "run_coverage.sh")
    set(COVERAGE_SCRIPT_IN   "${CMAKE_CURRENT_SOURCE_DIR}/${COVERAGE_SCRIPT_NAME}.in")
    set(COVERAGE_SCRIPT      "${CMAKE_CURRENT_BINARY_DIR}/${COVERAGE_SCRIPT_NAME}")


    # === Сохраняем список файлов ===
    file(WRITE ${COVERAGE_LIST_FILE} "")
    foreach(f ${COVERAGE_FILES})
        file(APPEND ${COVERAGE_LIST_FILE} "${f}\n")
    endforeach()

    # === Генерируем скрипт ===
    configure_file(
        "${COVERAGE_SCRIPT_IN}"
        "${COVERAGE_SCRIPT}"
        @ONLY
    )

    file(CHMOD ${COVERAGE_SCRIPT}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

    message(STATUS "Coverage script generated: ${COVERAGE_SCRIPT}")

endif()

