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

UpsEmulator::~UpsEmulator() {
    stop();
}

bool UpsEmulator::selectModel(const IniSectionName& name) {
    m_lastError.clear();

    // Проверяем запущен или нет
    if (m_running.load()) {
        m_lastError = "Cannot change model while emulator is running. Call stop() first.";
        return false;
    }

    // Проверяем, присутствует ли модель в списке доступных
    if (std::find(m_availableModels.begin(), m_availableModels.end(), name) ==
        m_availableModels.end()) {
        m_lastError = "Model not found: " + name;
        return false;
    }

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

    printf("Model selected: %s\n", name.c_str());
    return true;
}

bool UpsEmulator::fillDefaults() {
    const UpsOids& oids = m_config.oids();
    ErrorMessage err;

    // ---- modelName ----
    if (!m_store.set(oids.modelNameOID, m_config.modelName(), &err)) {
        m_lastError = "modelName: " + err;
        return false;
    }

    // Входные параметры InputStatus
    if (!m_store.set(oids.inputVoltageOID, "230", &err)) {
        m_lastError = "inputVoltage: " + err;
        return false;
    }
    if (!m_store.set(oids.inputFreqOID, "500", &err)) {
        m_lastError = "inputFrequency: " + err;
        return false;
    }

    // Выходные параметры OutputStatus
    if (!m_store.set(oids.outputVoltageOID, "220", &err)) {
        m_lastError = "outputVoltage: " + err;
        return false;
    }

    // Состояние батареи BatteryStatus
    if (!m_store.set(oids.batteryTempOID, "25", &err)) {
        m_lastError = "batteryTemperature: " + err;
        return false;
    }
    if (!m_store.set(oids.chargeRemainingOID, "100", &err)) {
        m_lastError = "chargeRemaining: " + err;
        return false;
    }

    // batteryStatus: первый элемент набора
    if (m_config.definedFields().batteryStatusSet.nameToValue.empty()) {
        m_lastError = "batteryStatusValues set is empty.";
        return false;
    }
    {
        const std::string& first =
            m_config.definedFields().batteryStatusSet.nameToValue.begin()->first;
        if (!m_store.set(oids.batteryStatusOID, first, &err)) {
            m_lastError = "batteryStatus: " + err;
            return false;
        }
    }

    // Состояние выхода OutputStatus
    // outputStatus: первый элемент набора
    if (m_config.definedFields().outputStatusSet.nameToValue.empty()) {
        m_lastError = "outputStatusValues set is empty.";
        return false;
    }
    {
        const std::string& first =
            m_config.definedFields().outputStatusSet.nameToValue.begin()->first;
        if (!m_store.set(oids.outputStatusOID, first, &err)) {
            m_lastError = "outputStatus: " + err;
            return false;
        }
    }

    return true;
}

bool UpsEmulator::start() {
    m_lastError.clear();

    // 1. Проверка, что модель выбрана
    if (m_currentModel.empty()) {
        m_lastError = "UPS model is not selected.";
        return false;
    }

    // 2. Привязка SNMP-агента к порту
    constexpr int SNMP_PORT = 161;
    if (!m_agent.bind(SNMP_PORT)) {
        m_lastError = "Failed to bind SNMP agent to port " +
                      std::to_string(SNMP_PORT);
        return false;
    }

    // 3. Запуск агента
    if (m_running.load()) {
        return false;
    }
    m_running.store(true);
    m_thread = std::thread(&UpsEmulator::runAgent, this);
    return true;
}

void UpsEmulator::stop() {
    if (!m_running.load()) {
        return;
    }
    m_agent.stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_running.store(false);
}

void UpsEmulator::runAgent() {
    m_agent.run();
    m_running.store(false);
}

bool UpsEmulator::isRunning() const {
    return m_running.load();
}

bool UpsEmulator::ok() const { return m_lastError.empty(); }

const ErrorMessage& UpsEmulator::lastError() const { return m_lastError; }

const std::vector<IniSectionName>& UpsEmulator::availableModels() const {
    return m_availableModels;
}
