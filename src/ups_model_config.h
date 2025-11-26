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

struct EnumMap {
    std::map<std::string, int> nameToValue;
    std::map<int, std::string> valueToName;
};

struct UpsEnums {
    EnumMap outputStatus;
    EnumMap batteryStatus;
};


class UpsModelConfig {
public:
    // загрузка конфигурации из файла path (абсолютного или относительного)
    bool load(const std::string& path, const std::string& section);

    const std::string& modelName() const;
    const UpsOids& oids() const;
    const UpsEnums& enums() const;
    const std::string& lastError() const;

private:
    std::string m_modelName{};
    UpsOids m_oids{};
    UpsEnums m_enums{};
    std::string m_lastError{};

    // проверка обязательных полей
    bool validate(const std::string& section);
    bool parseEnumMap(const std::string& raw, EnumMap& out);
};

#endif  // UPS_MODEL_CONFIG_H
