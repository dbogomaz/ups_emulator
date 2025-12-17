#ifndef UPS_EMULATOR_H
#define UPS_EMULATOR_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "ini_section_reader.h"
#include "snmp_agent.h"
#include "ups_data_store.h"
#include "ups_model_config.h"
#include "ups_types.h"

class UpsEmulator {
public:
    explicit UpsEmulator(const std::string& configPath);
    ~UpsEmulator();

    const std::vector<IniSectionName>& availableModels() const;
    const std::vector<std::string>& availableBatteryStatuses() const;
    const std::vector<std::string>& availableOutputStatuses() const;

    // Выбор модели UPS
    bool selectModel(const IniSectionName& name);
    IniSectionName currentModel() const;

    // Запуск эмулятора
    bool start();
    void stop();
    bool isRunning() const;

    bool ok() const;
    const ErrorMessage& lastError() const;

private:
    std::string m_configPath{};
    std::vector<IniSectionName> m_availableModels{};
    IniSectionName m_currentModel{};
    std::vector<std::string> m_availableBatteryStatuses{};
    std::vector<std::string> m_availableOutputStatuses{};

    UpsModelConfig m_config{};
    UpsDataStore m_store{};
    SnmpAgent m_agent{nullptr};

    ErrorMessage m_lastError{};

    std::thread m_thread;
    std::atomic<bool> m_running{false};

    // Основной цикл агента
    void runAgent();

    bool fillDefaults();
};

#endif  // UPS_EMULATOR_H
