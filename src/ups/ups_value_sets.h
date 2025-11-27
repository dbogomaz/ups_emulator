#ifndef UPS_VALUE_SETS_H
#define UPS_VALUE_SETS_H

#include <map>
#include <string>

// Описание значений сложного поля секции INI (например, batteryStatusValues)
struct FieldValueSet {
    std::map<std::string, int> nameToValue;
    std::map<int, std::string> valueToName;
};

// Набор структурированных полей, имеющих список значений
struct FieldValueSets {
    FieldValueSet batteryStatusSet;
    FieldValueSet outputStatusSet;
};

#endif  // UPS_VALUE_SETS_H