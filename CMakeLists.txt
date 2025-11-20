cmake_minimum_required(VERSION 3.10.0)

project(ups_emulator)

set(PROJECT_SRC
    main.cpp
    ups_model_config.cpp
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