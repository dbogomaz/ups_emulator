#ifndef UPS_OIDS_H
#define UPS_OIDS_H

#include <string>

#include "ups_types.h"

// Набор всех OID для UPS
struct UpsOids {
    Oid modelNameOID;
    Oid inputVoltageOID;
    Oid inputFreqOID;
    Oid outputVoltageOID;
    Oid batteryStatusOID;
    Oid chargeRemainingOID;
    Oid batteryTempOID;
    Oid outputStatusOID;
};

#endif  // UPS_OIDS_H