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

UpsEmulator::~UpsEmulator() { stop(); }

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
        // LCOV_EXCL_START
        // Не покрывается тестами, так как init() в текущей реализации всегда успешен
        m_lastError = "UpsDataStore initialization failed.";
        return false;
        // LCOV_EXCL_STOP
    }

    // Запоминаем текущую модель
    m_currentModel = name;

    // Выставляем дефолтные значения
    fillDefaults();

    // Заполняем доступные статусы батареи и выхода
    m_availableBatteryStatuses.clear();
    for (const auto& pair : m_config.definedFields().batteryStatusSet.nameToValue) {
        m_availableBatteryStatuses.push_back(pair.first);
    }

    m_availableOutputStatuses.clear();
    for (const auto& pair : m_config.definedFields().outputStatusSet.nameToValue) {
        m_availableOutputStatuses.push_back(pair.first);
    }

    printf("Model selected: %s\n", name.c_str());
    return true;
}

void UpsEmulator::fillDefaults() {
    const UpsOids& oids = m_config.oids();
    ErrorMessage err;

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
    const auto& batterySet = m_config.definedFields().batteryStatusSet.nameToValue;
    m_store.set(oids.batteryStatusOID, batterySet.begin()->first);

    // Состояние выхода OutputStatus
    // outputStatus: первый элемент набора
    const auto& outputSet = m_config.definedFields().outputStatusSet.nameToValue;
    m_store.set(oids.outputStatusOID, outputSet.begin()->first);
}

bool UpsEmulator::start(uint16_t port) {
    m_lastError.clear();

    // 1. Проверка, что модель выбрана
    if (m_currentModel.empty()) {
        m_lastError = "UPS model is not selected.";
        return false;
    }

    // 2. Проверка, что эмулятор не запущен
    if (m_running.load()) {
        m_lastError = "Emulator is already running.";
        return false;
    }

    // 3. Привязка SNMP-агента к порту
    if (!m_agent.bind(port)) {
        m_lastError = "Failed to bind SNMP agent to port " + std::to_string(port);
        return false;
    }

    // 4. Запуск основного цикла агента в отдельном потоке
    m_running.store(true);
    m_thread = std::thread(&UpsEmulator::runAgent, this);
    return true;
}

void UpsEmulator::stop() {
    if (!m_running.load()) {
        return;
    }

    // Ждём, пока агент реально войдёт в run()
    const auto waitStartTime = std::chrono::steady_clock::now();
    while (!m_agent.isRunning()) {
        // LCOV_EXCL_START
        if (std::chrono::steady_clock::now() - waitStartTime > std::chrono::milliseconds(500)) {
            break;
        }
        // LCOV_EXCL_STOP
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
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

bool UpsEmulator::isRunning() const { return m_running.load(); }

bool UpsEmulator::ok() const { return m_lastError.empty(); }

const ErrorMessage& UpsEmulator::lastError() const { return m_lastError; }

const std::vector<IniSectionName>& UpsEmulator::availableModels() const {
    return m_availableModels;
}

IniSectionName UpsEmulator::currentModel() const { return m_currentModel; }

const std::vector<std::string>& UpsEmulator::availableBatteryStatuses() const {
    return m_availableBatteryStatuses;
}

const std::vector<std::string>& UpsEmulator::availableOutputStatuses() const {
    return m_availableOutputStatuses;
}

bool UpsEmulator::setInputVoltage(int v) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().inputVoltageOID, std::to_string(v), &err)) {
        m_lastError = "setInputVoltage: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setInputFrequency(int hz) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().inputFreqOID, std::to_string(hz), &err)) {
        m_lastError = "setInputFrequency: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setChargeRemaining(int percent) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().chargeRemainingOID, std::to_string(percent), &err)) {
        m_lastError = "setChargeRemaining: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setBatteryTemperature(int celsius) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().batteryTempOID, std::to_string(celsius), &err)) {
        m_lastError = "setBatteryTemperature: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setBatteryStatus(const std::string& status) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().batteryStatusOID, status, &err)) {
        m_lastError = "setBatteryStatus: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setOutputVoltage(int v) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().outputVoltageOID, std::to_string(v), &err)) {
        m_lastError = "setOutputVoltage: " + err;
        return false;
    }
    return true;
}

bool UpsEmulator::setOutputStatus(const std::string& status) {
    ErrorMessage err;
    if (!m_store.set(m_config.oids().outputStatusOID, status, &err)) {
        m_lastError = "setOutputStatus: " + err;
        return false;
    }
    return true;
}
