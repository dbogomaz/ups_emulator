#include <iostream>

#include "ups_model_config.h"

int main() {
    UpsModelConfig cfg;

    if (!cfg.load("config/ups_models.ini", "APC")) {
        std::cout << "Failed to load config\n";
        std::cout << "Error: " << cfg.lastError() << "\n";
        return 1;
    }

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
    const EnumMap& bat = cfg.enums().batteryStatus;
    for (std::map<std::string, int>::const_iterator it = bat.nameToValue.begin();
         it != bat.nameToValue.end(); ++it) {
        std::cout << "  " << it->first << " = " << it->second << "\n";
    }
    std::cout << "\n";

    // ===== OutputStatusEnum =====
    std::cout << "OutputStatusEnum:\n";
    const EnumMap& out = cfg.enums().outputStatus;
    for (std::map<std::string, int>::const_iterator it = out.nameToValue.begin();
         it != out.nameToValue.end(); ++it) {
        std::cout << "  " << it->first << " = " << it->second << "\n";
    }
    std::cout << "\n";

    return 0;
}
