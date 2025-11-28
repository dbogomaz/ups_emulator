#include <iostream>

#include "ups_data_store.h"
#include "ups_model_config.h"

int main() {
    UpsModelConfig cfg;

    if (!cfg.load("config/ups_models.ini", "APC")) {
        std::cout << "Failed to load config\n";
        std::cout << "Error: " << cfg.lastError() << "\n";
        return 1;
    }
#if 0
    std::cout << "=== UPS Model Config Loaded ===\n\n";

    std::cout << "Model name           : " << cfg.modelName() << "\n";
    std::cout << "Model name OID       : " << cfg.oids().modelNameOID << "\n\n";

    std::cout << "Input voltage OID    : " << cfg.oids().inputVoltageOID << "\n";
    std::cout << "Input freq OID       : " << cfg.oids().inputFreqOID << "\n\n";

    std::cout << "Output voltage OID   : " << cfg.oids().outputVoltageOID << "\n";
    std::cout << "Output status OID    : " << cfg.oids().outputStatusOID << "\n\n";

    std::cout << "Battery status OID   : " << cfg.oids().batteryStatusOID << "\n";
    std::cout << "Charge remaining OID : " << cfg.oids().chargeRemainingOID << "\n";
    std::cout << "Battery temp OID     : " << cfg.oids().batteryTempOID << "\n\n";

    // ===== BatteryStatusEnum =====
    std::cout << "BatteryStatusEnum:\n";
    const FieldValueSet& bat = cfg.definedFields().batteryStatusSet;
    for (std::map<std::string, int>::const_iterator it = bat.nameToValue.begin();
         it != bat.nameToValue.end(); ++it) {
        std::cout << "  " << it->first << " = " << it->second << "\n";
    }
    std::cout << "\n";

    // ===== OutputStatusEnum =====
    std::cout << "OutputStatusEnum:\n";
    const FieldValueSet& out = cfg.definedFields().outputStatusSet;
    for (std::map<std::string, int>::const_iterator it = out.nameToValue.begin();
         it != out.nameToValue.end(); ++it) {
        std::cout << "  " << it->first << " = " << it->second << "\n";
    }
    std::cout << "\n";
#endif

    // ========================== Инициализация UpsDataStore ==========================
    UpsDataStore store;
    if (!store.init(cfg)) {
        std::cout << "Failed to init UpsDataStore\n";
        return 1;
    }

    auto printParameter = [&store](const Oid& oid) {
        const UpsParameter* p = store.get(oid);
        if (p) {
            std::cout << "Parameter: " << p->name << " OID: " << p->oid << " value: " << p->value
                      << "\n";
        } else {
            std::cout << "Parameter with OID " << oid << " not found\n";
        }
    };

    Oid testOid;
    ErrorMessage err;

    auto testSetParameter = [&store, &printParameter, &err](const Oid& oid,
                                                            const UpsParameterValue& value) {
        store.set(oid, value, &err);
        printf("Setting value: \"%s\"\n", value.c_str());
        printParameter(oid);
        if (!err.empty()) {
            std::cout << "Error setting parameter: " << err << "\n";
            err.clear();
        }
        printf("\n");
    };

    // testSetParameter(cfg.oids().modelNameOID, "New Model 123");
    // testSetParameter(cfg.oids().inputVoltageOID, "220");
    // testSetParameter(cfg.oids().inputFreqOID, "50");
    // testSetParameter(cfg.oids().outputVoltageOID, "bad_value_not_int");  // Ошибка
    testSetParameter(cfg.oids().batteryStatusOID, "batteryNormal");
    testSetParameter(cfg.oids().batteryStatusOID, "bad_value");  // Ошибка
    testSetParameter(cfg.oids().batteryStatusOID, "2");
    testSetParameter(cfg.oids().batteryStatusOID, "0"); // Ошибка




    return 0;
}
