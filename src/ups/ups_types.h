#ifndef UPS_TYPES_H
#define UPS_TYPES_H

#include <string>

// ------------------------------------------------------------------
// Базовые типы подсистемы UPS
// ------------------------------------------------------------------

// SNMP OID (идентификатор параметра)
using Oid = std::string;

// Значение параметра UPS (храним в строковом виде)
using UpsParameterValue = std::string;

// Имя параметра UPS (например "batteryStatus")
using UpsParameterName = std::string;

// Название модели UPS
using ModelName = std::string;

// Имя секции INI-файла
using IniSectionName = std::string;

// Сообщение об ошибке
using ErrorMessage = std::string;

// Тип параметра UPS
enum class UpsParameterType {
    Integer,   // Числовой параметр (напряжение, частота, температура)
    String     // Текстовый параметр (имя модели)
};

#endif // UPS_TYPES_H
