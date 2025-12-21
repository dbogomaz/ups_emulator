/**
 * @file ups_emulator.h
 * @brief Центральный класс эмулятора источника бесперебойного питания (UPS).
 *
 * Класс UpsEmulator управляет жизненным циклом эмулятора,
 * загружает конфигурацию моделей UPS, инициализирует SNMP-агент
 * и обеспечивает доступ к управлению параметрами устройства.
 *
 * Является точкой интеграции доменной модели UPS,
 * SNMP-подсистемы и пользовательских интерфейсов (CLI, GUI).
 */
#ifndef UPS_EMULATOR_H
#define UPS_EMULATOR_H

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "ini_section_reader.h"
#include "snmp_agent.h"
#include "ups_data_store.h"
#include "ups_model_config.h"
#include "ups_types.h"

/**
 * @class UpsEmulator
 * @brief Центральный класс управления эмулятором UPS.
 *
 * UpsEmulator отвечает за:
 * - загрузку конфигурации моделей UPS из INI-файла;
 * - выбор и инициализацию активной модели;
 * - управление жизненным циклом SNMP-агента;
 * - хранение и обновление текущего состояния UPS;
 * - предоставление API для внешних интерфейсов (CLI, GUI).
 *
 * Класс инкапсулирует взаимодействие между подсистемами
 * и предоставляет простой интерфейс управления эмулятором.
 */
class UpsEmulator {
public:
    /**
     * @brief Создаёт эмулятор UPS.
     *
     * @param configPath Путь к конфигурационному INI-файлу моделей UPS.
     */
    explicit UpsEmulator(const std::string& configPath);

    /**
     * @brief Останавливает эмулятор и освобождает ресурсы.
     */
    ~UpsEmulator();

    /**
     * @brief Возвращает список доступных моделей UPS.
     */
    const std::vector<IniSectionName>& availableModels() const;

    /**
     * @brief Выбирает активную модель UPS.
     *
     * @param name Имя секции INI-файла.
     * @return true при успешной инициализации модели.
     */
    bool selectModel(const IniSectionName& name);

    /**
     * @brief Возвращает имя текущей модели UPS.
     */
    IniSectionName currentModel() const;

    /**
     * @brief Возвращает список доступных статусов батареи.
     */
    const std::vector<std::string>& availableBatteryStatuses() const;

    /**
     * @brief Возвращает список доступных статусов выхода.
     */
    const std::vector<std::string>& availableOutputStatuses() const;

    /**
     * @brief Запускает эмулятор и SNMP-агент.
     *
     * @param port UDP-порт для SNMP-агента.
     * @return true при успешном запуске.
     */
    bool start(uint16_t port = 161);

    /**
     * @brief Останавливает эмулятор.
     */
    void stop();

    /**
     * @brief Проверяет, запущен ли эмулятор.
     */
    bool isRunning() const;

    /**
     * @brief Возвращает признак корректной инициализации эмулятора.
     */
    bool ok() const;

    /**
     * @brief Возвращает описание последней ошибки.
     */
    const ErrorMessage& lastError() const;

    /**
     * @brief Устанавливает значения параметров UPS.
     *
     * Используется внешними интерфейсами (CLI, GUI)
     * для изменения текущего состояния эмулируемого устройства.
     *
     * @return true при успешной установке значения.
     */

    bool setInputVoltage(int v);
    bool setInputFrequency(int hz);
    bool setChargeRemaining(int percent);
    bool setBatteryTemperature(int celsius);
    bool setBatteryStatus(const std::string& status);
    bool setOutputVoltage(int v);
    bool setOutputStatus(const std::string& status);

private:
    std::string m_configPath{};  ///< Путь к INI-файлу конфигурации моделей UPS.
    std::vector<IniSectionName> m_availableModels{};        ///< Список доступных моделей UPS.
    IniSectionName m_currentModel{};                        ///< Имя текущей модели UPS.
    std::vector<std::string> m_availableBatteryStatuses{};  ///< Список доступных статусов батареи.
    std::vector<std::string> m_availableOutputStatuses{};   ///< Список доступных статусов выхода.

    UpsModelConfig m_config{};     ///< Конфигурация текущей модели UPS.
    UpsDataStore m_store{};        ///< Хранилище параметров UPS.
    SnmpAgent m_agent{ nullptr };  ///< SNMP-агент эмулятора.

    ErrorMessage m_lastError{};  ///< Текст последней ошибки.

    std::thread m_thread;                  ///< Поток выполнения SNMP-агента.
    std::atomic<bool> m_running{ false };  ///< Признак запущенного эмулятора.

    /// Основной цикл работы SNMP-агента (выполняется в отдельном потоке).
    void runAgent();

    /// Заполняет хранилище UPS начальными значениями параметров модели.
    void fillDefaults();
};

#endif  // UPS_EMULATOR_H
