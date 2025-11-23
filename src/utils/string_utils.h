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
} // namespace utils

#endif // STRING_UTILS_H