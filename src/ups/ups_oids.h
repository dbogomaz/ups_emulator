/**
 * @file ups_oids.h
 * @brief Набор SNMP OID, используемых моделью UPS.
 *
 * Содержит структуру, агрегирующую все OID параметров,
 * поддерживаемых эмулятором источника бесперебойного питания.
 */
#ifndef UPS_OIDS_H
#define UPS_OIDS_H

#include <string>

#include "ups_types.h"

/// @ingroup ups

/**
 * @struct UpsOids
 * @brief Набор SNMP OID для параметров UPS.
 *
 * Структура используется для хранения соответствия между
 * логическими параметрами модели UPS и их SNMP OID.
 */
struct UpsOids {
    Oid modelNameOID;        ///< OID имени модели UPS.
    Oid inputVoltageOID;     ///< OID входного напряжения.
    Oid inputFreqOID;        ///< OID входной частоты.
    Oid outputVoltageOID;    ///< OID выходного напряжения.
    Oid batteryStatusOID;    ///< OID состояния батареи.
    Oid chargeRemainingOID;  ///< OID уровня заряда батареи.
    Oid batteryTempOID;      ///< OID температуры батареи.
    Oid outputStatusOID;     ///< OID состояния выхода.
};

#endif  // UPS_OIDS_H
