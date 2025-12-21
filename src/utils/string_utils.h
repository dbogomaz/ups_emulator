/**
 * @file string_utils.h
 * @brief Утилиты для работы со строками.
 *
 * Содержит вспомогательные функции для обработки строк
 * и чтения многострочных текстовых блоков.
 */
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <fstream>
#include <string>

/// @ingroup utils
namespace utils {

/**
 * @brief Удаляет пробельные символы в начале и конце строки.
 *
 * @param s Входная строка.
 * @return Строка без начальных и конечных пробельных символов.
 */
std::string trim(const std::string& s);

/**
 * @brief Считывает многострочный блок данных, заключённый в фигурные скобки.
 *
 * Объединяет первую строку и последующие строки файла
 * до момента, пока не будет встречена закрывающая фигурная скобка '}'.
 *
 * @param file Поток файла, продолжающийся после первой строки блока.
 * @param firstLine Первая строка блока (обычно содержащая '{').
 * @return Полный текст блока, объединённый в одну строку.
 */
std::string readMultilineBracedBlock(std::ifstream& file, const std::string& firstLine);

}  // namespace utils

#endif  // STRING_UTILS_H
