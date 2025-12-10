#include <iostream>

#include "snmp/snmp_agent.h"
#include "ups/ups_data_store.h"
#include "ups/ups_model_config.h"

int main() {
    // Загружаем модель (например, из файла ups_models.ini)
    UpsModelConfig cfg;
    if (!cfg.load("config/ups_models.ini", "APC")) {
        std::cerr << "Failed to load UPS model\n";
        return 1;
    }

    // Инициализируем хранилище параметров
    UpsDataStore store;
    if (!store.init(cfg)) {
        std::cerr << "Failed to init data store\n";
        return 1;
    }

    // устанавливаем значения
    store.set(cfg.oids().modelNameOID, cfg.modelName());
    // Входные параметры InputStatus
    store.set(cfg.oids().inputVoltageOID, "230");
    store.set(cfg.oids().inputFreqOID, "500");
    // Выходные параметры OutputStatus
    store.set(cfg.oids().outputVoltageOID, "220");
    // Состояние батареи BatteryStatus
    store.set(cfg.oids().batteryStatusOID,
              cfg.definedFields().batteryStatusSet.nameToValue.begin()->first);
    store.set(cfg.oids().batteryTempOID, "23");
    store.set(cfg.oids().chargeRemainingOID, "78");
    // Состояние выхода OutputStatus
    store.set(cfg.oids().outputStatusOID,
              cfg.definedFields().outputStatusSet.nameToValue.begin()->first);

    // Создаем SNMP-агент
    SnmpAgent agent(&store);

    if (!agent.start(161)) {
        std::cerr << "Failed to start SNMP agent\n";
        return 1;
    }

    // Переходим в основной цикл
    agent.run();

    return 0;
}