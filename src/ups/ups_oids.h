#ifndef UPS_OIDS_H
#define UPS_OIDS_H

#include <string>

// Набор всех OID для UPS
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

#endif  // UPS_OIDS_H