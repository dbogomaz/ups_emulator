#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

namespace utils {
/**
 * @brief Удаляет пробельные символы в начале и конце строки
 * @param s Входная строка
 * @return Обрезанная строка
 */
std::string trim(const std::string& s);

/**
 * @brief Считывает многострочный блок enum до символа '}'.
 * 
 * Объединяет firstLine и последующие строки файла,
 * пока не встретится закрывающая фигурная скобка.
 *
 * @param file Поток файла, продолжающийся после firstLine.
 * @param firstLine Первая строка блока (начинается с '{').
 * @return std::string Полный объединённый блок enum в одной строке.
 */
std::string readMultilineEnum(std::ifstream& file, const std::string& firstLine);

}  // namespace utils

#endif  // STRING_UTILS_H