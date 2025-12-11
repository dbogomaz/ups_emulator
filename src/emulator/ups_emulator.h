#ifndef UPS_EMULATOR_H
#define UPS_EMULATOR_H

#include <string>
#include <vector>

#include "ini_section_reader.h"
#include "snmp_agent.h"
#include "ups_data_store.h"
#include "ups_model_config.h"
#include "ups_types.h"

class UpsEmulator {
public:
    explicit UpsEmulator(const std::string& configPath);

    const std::vector<IniSectionName>& availableModels() const;

    // Выбор модели UPS
    bool selectModel(const IniSectionName& name);

    // Управление SNMP-агентом
    bool bind(int port);  // открыть сокет, привязать к порту
    bool stop();          // закрыть сокет, остановить
    void run();           // основной цикл

    bool ok() const;
    const ErrorMessage& lastError() const;

private:
    std::string m_configPath{};
    std::vector<IniSectionName> m_availableModels{};
    IniSectionName m_currentModel{};

    UpsModelConfig m_config{};
    UpsDataStore m_store{};
    SnmpAgent m_agent{nullptr};

    ErrorMessage m_lastError{};

    bool fillDefaults();
};

#endif  // UPS_EMULATOR_H
