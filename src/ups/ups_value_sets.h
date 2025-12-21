/**
 * @file ups_value_sets.h
 * @brief Описания наборов допустимых значений параметров UPS.
 *
 * Содержит структуры для представления параметров UPS,
 * принимающих ограниченный набор перечислимых значений,
 * и их отображения между строковыми именами и числовыми кодами.
 */
#ifndef UPS_VALUE_SETS_H
#define UPS_VALUE_SETS_H

#include <map>
#include <string>

/// @ingroup ups

/**
 * @struct FieldValueSet
 * @brief Набор соответствий между именами и числовыми значениями параметра.
 *
 * Используется для параметров UPS, имеющих фиксированный набор
 * допустимых значений (например, состояния батареи или выхода).
 */
struct FieldValueSet {
    std::map<std::string, int> nameToValue;  ///< Отображение: имя → числовое значение.
    std::map<int, std::string> valueToName;  ///< Отображение: числовое значение → имя.
};

/**
 * @struct FieldValueSets
 * @brief Набор всех перечислимых параметров UPS.
 *
 * Содержит структуры значений для параметров UPS,
 * поддерживающих перечислимые состояния.
 */
struct FieldValueSets {
    FieldValueSet batteryStatusSet;  ///< Набор значений состояния батареи.
    FieldValueSet outputStatusSet;   ///< Набор значений состояния выхода.
};

#endif  // UPS_VALUE_SETS_H
