#ifndef INI_SECTION_READER_H
#define INI_SECTION_READER_H

#include <string>
#include <vector>

#include "ups_types.h"

namespace utils {

// Класс для извлечения списка секций INI-файла.
class IniSectionReader {
public:
    explicit IniSectionReader(const std::string& path);

    // Успешно ли прошло чтение
    bool ok() const;

    // Список найденных секций INI-файла (в порядке появления)
    const std::vector<IniSectionName>& sections() const;

    // Последняя ошибка (пустая строка при отсутствии ошибок)
    const ErrorMessage& lastError() const;

private:
    bool m_ok = false;                       // Флаг успешности операции
    std::string m_path;                      // Путь к INI-файлу
    std::vector<IniSectionName> m_sections;  // Список секций
    ErrorMessage m_lastError;                // Текст ошибки

    void scan();
};

}  // namespace utils

#endif  // INI_SECTION_READER_H
