#!/bin/bash
# @file doc_generation.sh
set -eEuo pipefail

# Абсолютный путь к каталогу скрипта
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

# Запуск Doxygen
doxygen Doxyfile

# Подсчёт warnings
WARN_FILE="doxygen_warnings.log"
WARN_COUNT=0

if [[ -f "$WARN_FILE" ]]; then
    WARN_COUNT=$(grep -c "warning:" "$WARN_FILE" || true)
fi

# Цвета
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
RESET="\033[0m"

if [[ "$WARN_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}Doxygen: warnings = 0${RESET}"
else
    echo -e "${YELLOW}Doxygen: warnings = ${WARN_COUNT} (details in ${WARN_FILE})${RESET}"
fi
