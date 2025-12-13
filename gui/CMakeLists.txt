# @file gui/CMakeLists.txt

set(CMAKE_AUTOMOC ON)

if(NOT Qt5_DIR)
    set(Qt5_DIR /Users/bgm/Qt/5.15.2/clang_64/lib/cmake/Qt5)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(ups_emulator_gui
    main.cpp
    main_window.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(ups_emulator_gui
    Qt5::Widgets
)
