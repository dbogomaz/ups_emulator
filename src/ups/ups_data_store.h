#ifndef UPS_DATA_STORE_H
#define UPS_DATA_STORE_H

#include <unordered_map>

#include "ups_parameter.h"
#include "ups_model_config.h"

// Хранилище текущего состояния UPS
class UpsDataStore {
public:
    // Инициализация хранилища по конфигурации
    bool init(const UpsModelConfig& cfg);

    // Проверить, существует ли параметр с данным OID
    bool has(const Oid& oid) const;

    // Получить параметр по OID 
    bool get(const Oid& oid, UpsParameter& out) const;

    // Установить новое значение параметра
    bool set(const Oid& oid, const UpsParameterValue& value, ErrorMessage* err = nullptr);

private:
    // Параметры UPS по ключу OID
    std::unordered_map<Oid, UpsParameter> m_parameters{};

    // Ограничения для перечислимых параметров (OID -> набор значений)
    std::unordered_map<Oid, const FieldValueSet*> m_valueSets{};
};

#endif // UPS_DATA_STORE_H
