#!/bin/bash
# @file run_clang_tidy.sh
set -eEuo pipefail

# ---- Установка конфигурации ----
set_config() {
    BUILD_DIR=build

    # Режим clang-tidy:
    #   light   — минимальный, почти без шума (по умолчанию)
    #   medium  — расширенный анализ
    #   strict  — жёсткий аудит
    CLANG_TIDY_MODE=${CLANG_TIDY_MODE:-light}
}

# ---- Проверка условий ----
check_environment() {
    # Проверяем, что есть compile_commands.json
    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        echo "ERROR: ${BUILD_DIR}/compile_commands.json not found."
        echo "Run: cmake -S . -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        exit 1
    fi
}

# ---- Определение наборов проверок clang-tidy ----
set_clang_tidy_checks() {
    # ------------------------------------------------------------
    # CLANG_TIDY_CHECKS_LIGHT
    #
    # Назначение:
    #   Минимальный набор реальных дефектов.
    #   Можно запускать регулярно, почти без ложных срабатываний.
    #
    # Рекомендуется:
    #   • первый запуск
    #   • регулярная проверка перед релизом
    # ------------------------------------------------------------
    CLANG_TIDY_CHECKS_LIGHT=(
        bugprone-unused-*
        bugprone-empty-catch
        bugprone-narrowing-conversions
    )

    # ------------------------------------------------------------
    # CLANG_TIDY_CHECKS_MEDIUM
    #
    # Назначение:
    #   Расширенный анализ:
    #   • потенциальные ошибки
    #   • неочевидные проблемы с параметрами
    #   • базовые performance-рекомендации
    #
    # Рекомендуется:
    #   • ручной запуск
    #   • разбор по одному файлу
    # ------------------------------------------------------------
    CLANG_TIDY_CHECKS_MEDIUM=(
        bugprone-unused-*
        bugprone-empty-catch
        bugprone-narrowing-conversions
        bugprone-easily-swappable-parameters
        performance-unnecessary-copy-initialization
        performance-unnecessary-value-param
    )

    # ------------------------------------------------------------
    # CLANG_TIDY_CHECKS_STRICT
    #
    # Назначение:
    #   Жёсткий аудит кода.
    #   Может выдавать много шума и архитектурных замечаний.
    #
    # НЕ рекомендуется:
    #   • для регулярного использования
    #   • для автоматических проверок
    #
    # Использовать:
    #   • эпизодически
    #   • осознанно
    # ------------------------------------------------------------
    CLANG_TIDY_CHECKS_STRICT=(
        bugprone-*
        performance-*
        portability-*
    )

    # NOTE:
    # readability-*  — намеренно отключены (дублируют clang-format, много шума)
    # modernize-*    — намеренно отключены (проект C++11, важна совместимость)
    # cppcoreguidelines-* — намеренно отключены (слишком жёсткие правила)
    # misc-*         — намеренно отключены (слишком много ложных срабатываний)
}

# ---- Выбор профиля clang-tidy по режиму ----
select_clang_tidy_profile() {
    case "${CLANG_TIDY_MODE}" in
        light)
            CLANG_TIDY_CHECKS=("${CLANG_TIDY_CHECKS_LIGHT[@]}")
            ;;
        medium)
            CLANG_TIDY_CHECKS=("${CLANG_TIDY_CHECKS_MEDIUM[@]}")
            ;;
        strict)
            CLANG_TIDY_CHECKS=("${CLANG_TIDY_CHECKS_STRICT[@]}")
            ;;
        *)
            echo "ERROR: Unknown CLANG_TIDY_MODE='${CLANG_TIDY_MODE}'"
            echo "Valid values: light | medium | strict"
            exit 1
            ;;
    esac


    echo -e "\033[36mclang-tidy mode: ${CLANG_TIDY_MODE}\033[0m"
}

# --- Формирование флагов для clang-tidy
set_clang_tidy_flags() {
    CLANG_TIDY_FLAGS=(
        "-checks=$(IFS=,; echo "${CLANG_TIDY_CHECKS[*]}")"
        "-warnings-as-errors="
        "-header-filter=^src/.*"
    )
}

# ---- Определение целевых файлов для анализа ----
set_targets() {
    TARGETS=()

    # Если файлы переданы аргументами — используем их
    if [[ $# -gt 0 ]]; then
        for file in "$@"; do
            if [[ ! -f "$file" ]]; then
                echo "ERROR: File not found: $file"
                exit 1
            fi
            TARGETS+=("$file")
        done
        return
    fi

    # Иначе — анализируем все .cpp в src
    while IFS= read -r file; do
        TARGETS+=("$file")
    done < <(find src -type f -name '*.cpp')
}

# --- Запуск clang-tidy ---
run_clang_tidy() {
    clang-tidy \
        "${TARGETS[@]}" \
        "${CLANG_TIDY_FLAGS[@]}" \
        -p "${BUILD_DIR}"
}

main() {
    set_config
    check_environment
    set_clang_tidy_checks
    select_clang_tidy_profile
    set_clang_tidy_flags
    set_targets "$@"
    run_clang_tidy
}

main "$@"
