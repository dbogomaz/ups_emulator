#ifndef UPS_MODEL_CONFIG_H
#define UPS_MODEL_CONFIG_H

#include "ups_oids.h"
#include "ups_value_sets.h"

// Класс для загрузки и хранения конфигурации модели UPS из INI файла
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
