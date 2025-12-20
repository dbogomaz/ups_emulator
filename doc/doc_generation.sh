#!/bin/bash
# @file doc_generation.sh
set -eEuo pipefail

# Абсолютный путь к каталогу, где лежит скрипт
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "${SCRIPT_DIR}"

doxygen Doxyfile
