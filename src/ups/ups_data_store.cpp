#include "ups_data_store.h"

#include "utils/string_utils.h"

bool UpsDataStore::init(const UpsModelConfig& cfg, ErrorMessage* err) {
    m_parameters.clear();
    m_valueSets.clear();

    const UpsOids& oids = cfg.oids();

    // ---- Вспомогательная лямбда для добавления параметра ----
    auto addParam = [&](const Oid& oid, const UpsParameterName& name,
                        UpsParameterType type) -> bool {
        if (oid.empty()) {
            if (err) *err = "OID for field '" + name + "' is empty";
            return false;
        }

        UpsParameter p;
        p.name = name;
        p.oid = oid;
        p.type = type;

        // Значение по умолчанию
        p.value = (type == UpsParameterType::String ? "" : "0");

        m_parameters[oid] = p;
        return true;
    };

    // ---- Добавление всех базовых параметров ----
    if (!addParam(oids.modelNameOID, "modelName", UpsParameterType::String)) return false;
    if (!addParam(oids.inputVoltageOID, "inputVoltage", UpsParameterType::Integer)) return false;
    if (!addParam(oids.inputFreqOID, "inputFreq", UpsParameterType::Integer)) return false;
    if (!addParam(oids.outputVoltageOID, "outputVoltage", UpsParameterType::Integer)) return false;
    if (!addParam(oids.batteryStatusOID, "batteryStatus", UpsParameterType::Integer)) return false;
    if (!addParam(oids.chargeRemainingOID, "chargeRemaining", UpsParameterType::Integer))
        return false;
    if (!addParam(oids.batteryTempOID, "batteryTemp", UpsParameterType::Integer)) return false;
    if (!addParam(oids.outputStatusOID, "outputStatus", UpsParameterType::Integer)) return false;

    // ---- Добавление наборов допустимых значений (ValueSets) ----
    const FieldValueSets& sets = cfg.definedFields();

    if (!sets.batteryStatusSet.nameToValue.empty())
        m_valueSets[oids.batteryStatusOID] = &sets.batteryStatusSet;

    if (!sets.outputStatusSet.nameToValue.empty())
        m_valueSets[oids.outputStatusOID] = &sets.outputStatusSet;

#if 0 // ---- Отладочный вывод загруженных параметров ----
    printf("\n\n=== UpsDataStore Initialized ===\n");
    // Выводим по имени
    auto it = m_valueSets.begin();
    while (it != m_valueSets.end()) {
        printf("ValueSet for OID: %s\n", it->first.c_str());
        auto itFieldSet = it->second->nameToValue.begin();
        while (itFieldSet != it->second->nameToValue.end()) {
            printf("\t%s = %d\n", itFieldSet->first.c_str(), itFieldSet->second);
            ++itFieldSet;
        }
        ++it;
    }
    printf("\n");
    // Выводим по значению
    it = m_valueSets.begin();
    while (it != m_valueSets.end()) {
        printf("ValueSet for OID: %s\n", it->first.c_str());
        auto itFieldSet = it->second->valueToName.begin();
        while (itFieldSet != it->second->valueToName.end()) {
            printf("\t%d = %s\n", itFieldSet->first, itFieldSet->second.c_str());
            ++itFieldSet;
        }
        ++it;
    }
    printf("\n");
#endif

    return true;
}

bool UpsDataStore::has(const Oid& oid) const {
    return m_parameters.find(oid) != m_parameters.end();
}

const UpsParameter* UpsDataStore::get(const Oid& oid) const {
    auto it = m_parameters.find(oid);
    if (it != m_parameters.end()) return &it->second;
    return nullptr;
}

bool UpsDataStore::set(const Oid& oid, const UpsParameterValue& value, ErrorMessage* err) {
    // ---- 1. Найти параметр ----
    UpsParameter* p = nullptr;
    auto it = m_parameters.find(oid);
    if (it != m_parameters.end()) {
        p = &it->second;
    }
    if (!p) {
        if (err) *err = "Unknown OID: " + oid;
        return false;
    }

    // ---- 2. Проверка ValueSet (если есть ограничения) ----
    auto itSet = m_valueSets.find(oid);
    if (itSet != m_valueSets.end()) {
        const FieldValueSet* vset = itSet->second;
        // --- 2.1 Попробовать как ИМЯ ---
        auto itName = vset->nameToValue.find(value);
        if (itName != vset->nameToValue.end()) {
            // сохраним ЧИСЛО — это реальное значение SNMP
            p->value = std::to_string(itName->second);
            return true;
        }
        // --- 2.2 Попробовать как ЧИСЛО ---
        try {
            int numeric = std::stoi(value);
            auto itNum = vset->valueToName.find(numeric);
            if (itNum != vset->valueToName.end()) {
                p->value = value; // число валидно
                return true;
            }
        } catch (...) {
            // string not numeric
        }
        // --- 2.3 Ошибка: значение не подходит ---
        if (err) *err = "Invalid value for ValueSet parameter: " + value;
        return false;
    }

    // ---- 3. Integer (обычный) ----
    if (p->type == UpsParameterType::Integer) {
        try {
            std::stoi(value);
        } catch (...) {
            if (err) *err = "Invalid integer value: " + value;
            return false;
        }
        p->value = value;
        return true;
    }

    // ---- 4. String ----
    if (p->type == UpsParameterType::String) {
        p->value = value;
        return true;
    }

    return false;
}
