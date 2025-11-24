cmake_minimum_required(VERSION 3.10.0)

if(1) # PROJECT CONFIG BLOCK
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
endif()

if(1) # OPTIONS (TESTING, COVERAGE) BLOCK
    option(BUILD_TESTING "Build unit tests" ON)
    option(BUILD_COVERAGE "Enable coverage instrumentation" ON)
    if (BUILD_COVERAGE)
        set(BUILD_TESTING ON CACHE BOOL "Build unit tests" FORCE)
        set(COVERAGE_COMPILE_FLAGS --coverage -O0 -g)
        set(COVERAGE_LINK_FLAGS --coverage)
    endif()
endif()

if(1) # UPS CORE LIBRARY BLOCK
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
endif()

if(1) # MAIN APPLICATION BLOCK
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
endif()

if(1) # ACTIVATE BUILD TESTS BLOCK
    if (BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

if(1) # COPY REQUIRED FILES BLOCK
    set(CONFIG_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config)
    file(MAKE_DIRECTORY ${CONFIG_FOLDER})
    file(COPY ${CMAKE_SOURCE_DIR}/ups_models.ini DESTINATION ${CONFIG_FOLDER})
endif()