#ifndef UPS_MODEL_CONFIG_H
#define UPS_MODEL_CONFIG_H

#include <string>
#include <vector>
#include <map>

// Список OID полей для UPS
struct UpsOids {
    std::string modelNameOID;
    std::string inputVoltageOID;
    std::string inputFreqOID;
    std::string outputVoltageOID;
    std::string batteryStatusOID;
    std::string chargeRemainingOID;
    std::string batteryTempOID;
    std::string outputStatusOID;
};

// Описание значений сложного поля секции INI (например, batteryStatusValues)
struct FieldValueSet {
    std::map<std::string, int> nameToValue;
    std::map<int, std::string> valueToName;
};


// Набор структурированных полей, имеющих список значений
struct FieldValueSets {
    FieldValueSet outputStatusSet;
    FieldValueSet batteryStatusSet;
};

class UpsModelConfig {
public:
    // загрузка конфигурации из файла path (абсолютного или относительного)
    bool load(const std::string& path, const std::string& section);

    const std::string& modelName() const;
    const UpsOids& oids() const;
    const FieldValueSets& definedFields() const;
    const std::string& lastError() const;

private:
    std::string m_modelName{};
    UpsOids m_oids{};
    FieldValueSets m_definedFields{};
    std::string m_lastError{};

    // проверка обязательных полей
    bool validate(const std::string& section);
    bool parseFieldValueSet(const std::string& raw, FieldValueSet& out);
};

#endif  // UPS_MODEL_CONFIG_H
