# coverage/CMakeLists.txt
# ============================================================
#                    COVERAGE SUBSYSTEM
# ============================================================

message(STATUS "Configuring COVERAGE subsystem")

# Получаем исходники библиотеки
get_target_property(UPS_CORE_SOURCES ups_core SOURCES)

# Создаем список файлов для coverage
set(COVERAGE_LIST_FILE "${CMAKE_BINARY_DIR}/coverage_file_list.txt")
file(WRITE ${COVERAGE_LIST_FILE} "")
foreach(f ${UPS_CORE_SOURCES})
    get_filename_component(abs_f "${CMAKE_SOURCE_DIR}/${f}" REALPATH)
    file(APPEND ${COVERAGE_LIST_FILE} "${abs_f}\n")
endforeach()

# Генерация coverage-скрипта
set(COVERAGE_SCRIPT_NAME "run_coverage.sh")
set(COVERAGE_SCRIPT_IN   "${CMAKE_CURRENT_SOURCE_DIR}/run_coverage.sh.in")
set(COVERAGE_SCRIPT      "${CMAKE_BINARY_DIR}/${COVERAGE_SCRIPT_NAME}")
configure_file(
    ${COVERAGE_SCRIPT_IN}
    ${COVERAGE_SCRIPT}
    @ONLY
)
file(CHMOD ${COVERAGE_SCRIPT}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

message(STATUS "Coverage script generated: ${COVERAGE_SCRIPT}")
