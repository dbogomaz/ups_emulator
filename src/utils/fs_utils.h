#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <string>

namespace utils {
    /**
     * @brief Получает каталог исполняемого файла
     * @return Путь к каталогу исполняемого файла
     */
    std::string getBinaryDir();

    /**
     * @brief Преобразует путь к файлу относительно каталога 
     *        исполняемого файла в абсолютный путь
     * @param path Входной путь
     * @return Абсолютный путь к файлу
     */
    std::string resolvePath(const std::string& path);
} // namespace utils

#endif // FS_UTILS_H