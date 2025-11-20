#include <iostream>
#include "ups_model_config.h"

int main()
{
    UpsModelConfig cfg;
    if (!cfg.load("config/ups_models.ini", "APC")) {
        std::cout << "Failed to load config\n";
        return 1;
    }

    std::cout << "Model name: " << cfg.modelName << "\n";
    std::cout << "Model OID : " << cfg.oids.modelNameOID << "\n\n";

    std::cout << "Input voltage OID : " << cfg.oids.inputVoltageOID << "\n";
    std::cout << "Input freq OID    : " << cfg.oids.inputFreqOID << "\n\n";

    std::cout << "Output voltage OID: " << cfg.oids.outputVoltageOID << "\n\n";

    std::cout << "Battery status OID: " << cfg.oids.batteryStatusOID << "\n";
    std::cout << "Charge remain OID : " << cfg.oids.chargeRemainingOID << "\n";
    std::cout << "Battery temp OID  : " << cfg.oids.batteryTempOID << "\n\n";

    std::cout << "Bypass status OID : " << cfg.oids.bypassStatusOID << "\n";
    std::cout << "Bypass values     : ";
    for (int v : cfg.bypassValues) std::cout << v << " ";
    std::cout << "\n";

    return 0;
}
