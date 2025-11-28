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

    return true;
}

bool UpsDataStore::has(const Oid& oid) const { return false; }

const UpsParameter* UpsDataStore::get(const Oid& oid) const { return nullptr; }

bool UpsDataStore::set(const Oid& oid, const UpsParameterValue& value, ErrorMessage* err) {
    return false;
}
