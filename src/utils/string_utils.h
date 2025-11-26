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
 * @brief Считывает многострочный enum из файла, начиная с первой строки
 * 
 * @param file Поток файла для чтения
 * @param firstLine Первая строка enum
 * @return std::string Полный многострочный enum в виде строки
 */
std::string readMultilineEnum(std::ifstream& file, const std::string& firstLine);

}  // namespace utils

#endif  // STRING_UTILS_H