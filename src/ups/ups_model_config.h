#ifndef UPS_MODEL_CONFIG_H
#define UPS_MODEL_CONFIG_H

#include "ups_oids.h"
#include "ups_value_sets.h"

// Класс для загрузки и хранения конфигурации модели UPS из INI файла
class UpsModelConfig {
public:
    bool load(const std::string& path, const IniSectionName& section);

    const ModelName& modelName() const;
    const UpsOids& oids() const;
    const FieldValueSets& definedFields() const;
    const ErrorMessage& lastError() const;

private:
    ModelName m_modelName{};
    UpsOids m_oids{};
    FieldValueSets m_definedFields{};
    ErrorMessage m_lastError{};

    bool validate(const IniSectionName& section);
    bool parseFieldValueSet(const std::string& raw, FieldValueSet& out);
};

#endif  // UPS_MODEL_CONFIG_H
