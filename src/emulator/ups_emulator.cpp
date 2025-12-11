#include "ups_emulator.h"

#include <algorithm>
#include <cstdio>

#include "ini_section_reader.h"

UpsEmulator::UpsEmulator(const std::string& configPath)
    : m_configPath(configPath), m_agent(&m_store) {
    utils::IniSectionReader reader(configPath);

    if (!reader.ok()) {
        m_lastError = reader.lastError();
        return;
    }

    m_availableModels = reader.sections();
}

bool UpsEmulator::selectModel(const IniSectionName& name) {
    m_lastError.clear();

    // Проверяем, присутствует ли модель в списке доступных
    if (std::find(m_availableModels.begin(), m_availableModels.end(), name) ==
        m_availableModels.end()) {
        m_lastError = "Model not found: " + name;
        return false;
    }

    // TODO: Возможно нужно сначала оставновить
    // if (m_agent.isRunning()) {
    //     m_lastError = "Cannot change model while agent is running. Call stop() first.";
    //     return false;
    // }

    // Загружаем модель
    if (!m_config.load(m_configPath, name)) {
        m_lastError = m_config.lastError();
        return false;
    }

    // Инициализируем хранилище параметров
    if (!m_store.init(m_config)) {
        m_lastError = "UpsDataStore initialization failed.";
        return false;
    }

    // Запоминаем текущую модель
    m_currentModel = name;

    // Выставляем дефолтные значения
    if (!fillDefaults()) {
        // m_lastError заполнен в fillDefaults()
        return false;
    }

    return true;
}

bool UpsEmulator::fillDefaults() {
    const UpsOids& oids = m_config.oids();

    // ---- modelName ----
    m_store.set(oids.modelNameOID, m_config.modelName());

    // Входные параметры InputStatus
    m_store.set(oids.inputVoltageOID, "230");
    m_store.set(oids.inputFreqOID, "500");

    // Выходные параметры OutputStatus
    m_store.set(oids.outputVoltageOID, "220");

    // Состояние батареи BatteryStatus
    m_store.set(oids.batteryTempOID, "25");
    m_store.set(oids.chargeRemainingOID, "100");
    // batteryStatus: первый элемент набора
    if (!m_config.definedFields().batteryStatusSet.nameToValue.empty()) {
        const std::string& first =
            m_config.definedFields().batteryStatusSet.nameToValue.begin()->first;
        m_store.set(oids.batteryStatusOID, first);
    } else {
        m_lastError = "batteryStatusValues set is empty.";
        return false;
    }

    // Состояние выхода OutputStatus
    // outputStatus: первый элемент набора
    if (!m_config.definedFields().outputStatusSet.nameToValue.empty()) {
        const std::string& first =
            m_config.definedFields().outputStatusSet.nameToValue.begin()->first;
        m_store.set(oids.outputStatusOID, first);
    } else {
        m_lastError = "outputStatusValues set is empty.";
        return false;
    }

    return true;
}

bool UpsEmulator::bind(int port) {
    m_lastError.clear();

    if (!m_agent.bind(port)) {
        m_lastError = "Failed to bind SNMP agent to port " + std::to_string(port);
        return false;
    }

    return true;
}

bool UpsEmulator::stop() {
    m_lastError.clear();

    m_agent.stop();
    // TODO: возможно нужно возвращать bool вместо void SnmpAgent::stop()
    // if (!m_agent.stop()) {
    //     m_lastError = "Failed to stop SNMP agent.";
    //     return false;
    // }

    return true;
}

void UpsEmulator::run() { m_agent.run(); }

bool UpsEmulator::ok() const { return m_lastError.empty(); }

const ErrorMessage& UpsEmulator::lastError() const { return m_lastError; }

const std::vector<IniSectionName>& UpsEmulator::availableModels() const {
    return m_availableModels;
}
