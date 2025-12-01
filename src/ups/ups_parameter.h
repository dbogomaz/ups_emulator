#ifndef UPS_PARAMETER_H
#define UPS_PARAMETER_H

#include <string>
#include "ups_types.h"

/**
 * @brief Параметр UPS (единица данных в UpsDataStore).
 *
 * Данная структура описывает один параметр устройства UPS,
 * доступный через SNMP. Параметр содержит логическое имя,
 * строковый OID, тип данных, а также текущее значение.
 *
 * Все значения хранятся в строковом виде, поскольку SNMP
 * обменивается строками. При необходимости преобразования
 * (например, для целочисленных параметров) логика реализуется
 * в UpsDataStore.
 *
 * Примеры параметров:
 *  - inputVoltage (Integer)
 *  - batteryStatus (Integer, enumerated)
 *  - modelName (String)
 */
struct UpsParameter {
    /**
     * @brief Логическое имя параметра.
     *
     * Например: "batteryStatus", "inputVoltage", "modelName".
     */
    UpsParameterName name;

    /**
     * @brief SNMP OID параметра.
     *
     * Например: "1.3.6.1.4.1.318.1.1.1.3.2.1.0".
     */
    Oid oid;

    /**
     * @brief Тип параметра UPS.
     *
     * Указывает, является ли параметр строковым (String)
     * или числовым (Integer). Все значения всё равно хранятся
     * в строковом виде, однако тип помогает DataStore
     * корректно обрабатывать их.
     */
    UpsParameterType type;

    /**
     * @brief Текущее значение параметра (в строковом виде).
     *
     * Значение может содержать:
     *  - целое число ("230")
     *  - строку ("Smart-UPS RT 2000 XL")
     * 
     * UpsDataStore отвечает за корректность и валидацию.
     */
    UpsParameterValue value;
};

#endif // UPS_PARAMETER_H
