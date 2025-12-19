#!/bin/bash
# @file run_cppcheck.sh
set -eEuo pipefail

CPPCHECK_FLAGS=(
    --std=c++11                                       # Использовать стандарт C++11
    "--enable=warning,style,performance,portability"  # Категории: warning, style, performance, portability
    --suppress=missingIncludeSystem                   # Подавление предупреждений о недоступных системных include
    --check-level=exhaustive                          # Максимальный уровень глубины проверок
    --inconclusive                                    # Включение предположительных (inconclusive) проверок
    --inline-suppr                                    # Разрешить inline-подавления (// cppcheck-suppress)
    # --quiet                                           # Минимизировать вывод (только предупреждения и ошибки)
    -Isrc                                             # Добавить src в include-path
)

# Если аргументы не переданы — анализируем src
TARGETS=("$@")
if [[ ${#TARGETS[@]} -eq 0 ]]; then
    TARGETS=(src)
fi

cppcheck "${CPPCHECK_FLAGS[@]}" "${TARGETS[@]}"
