#!/bin/bash
# @file run_clang_tidy.sh
set -eEuo pipefail

BUILD_DIR=build

# Проверяем, что есть compile_commands.json
if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    echo "ERROR: ${BUILD_DIR}/compile_commands.json not found."
    echo "Run: cmake -S . -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    exit 1
fi

# Набор проверок clang-tidy

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

CLANG_TIDY_FLAGS=(
    # --- Выбрать ОДИН профиль ---
    "-checks=$(IFS=,; echo "${CLANG_TIDY_CHECKS_LIGHT[*]}")"
    # "-checks=$(IFS=,; echo "${CLANG_TIDY_CHECKS_MEDIUM[*]}")"
    # "-checks=$(IFS=,; echo "${CLANG_TIDY_CHECKS_STRICT[*]}")"

    "-warnings-as-errors="
    "-header-filter=^src/.*"
)

# Если аргументы не переданы — анализируем все .cpp в src
if [[ $# -eq 0 ]]; then
    mapfile -t TARGETS < <(find src -name '*.cpp')
else
    TARGETS=("$@")
fi

# Запуск clang-tidy
clang-tidy \
    "${TARGETS[@]}" \
    "${CLANG_TIDY_FLAGS[@]}" \
    -p "${BUILD_DIR}"
