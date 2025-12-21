/**
 * @file ups_types.h
 * @brief Базовые типы доменной модели UPS.
 *
 * Содержит фундаментальные типы и перечисления,
 * используемые в подсистеме эмуляции ИБП.
 */
#ifndef UPS_TYPES_H
#define UPS_TYPES_H

#include <string>

/// @ingroup ups

/**
 * @brief SNMP OID (идентификатор параметра).
 */
using Oid = std::string;

/**
 * @brief Значение параметра UPS (хранится в строковом виде).
 */
using UpsParameterValue = std::string;

/**
 * @brief Имя параметра UPS (например, "batteryStatus").
 */
using UpsParameterName = std::string;

/**
 * @brief Название модели UPS.
 */
using ModelName = std::string;

/**
 * @brief Имя секции INI-файла.
 */
using IniSectionName = std::string;

/**
 * @brief Сообщение об ошибке.
 */
using ErrorMessage = std::string;

/**
 * @enum UpsParameterType
 * @brief Тип параметра UPS.
 */
enum class UpsParameterType {
    /// Числовой параметр (напряжение, частота, температура).
    Integer,

    /// Текстовый параметр (например, имя модели).
    String
};

#endif  // UPS_TYPES_H
