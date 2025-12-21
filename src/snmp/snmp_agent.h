/**
 * @file snmp_agent.h
 * @brief SNMP-агент для обработки запросов к эмулятору UPS.
 *
 * Содержит класс, реализующий приём и обработку SNMP-запросов
 * по протоколу UDP и взаимодействие с хранилищем данных UPS.
 */
#ifndef SNMP_AGENT_H
#define SNMP_AGENT_H

#include <netinet/in.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "snmp_codec.h"
#include "ups_data_store.h"

/// @ingroup snmp

/**
 * @class SnmpAgent
 * @brief SNMP-агент, обслуживающий запросы к эмулятору UPS.
 *
 * Класс отвечает за:
 * - инициализацию и управление UDP-сокетом;
 * - приём SNMP-пакетов от клиентов;
 * - декодирование SNMP-запросов;
 * - формирование и отправку SNMP-ответов;
 * - взаимодействие с хранилищем данных UPS.
 *
 * Агент не владеет данными UPS и использует UpsDataStore
 * исключительно для чтения и изменения параметров.
 */
class SnmpAgent {
public:
    /**
     * @brief Создаёт SNMP-агент, связанный с хранилищем данных UPS.
     *
     * @param store Указатель на хранилище данных UPS.
     */
    explicit SnmpAgent(UpsDataStore* store);

    /**
     * @brief Инициализирует UDP-сокет и привязывает его к порту.
     *
     * @param port UDP-порт для приёма SNMP-запросов (по умолчанию 161).
     * @return true в случае успешной инициализации.
     */
    bool bind(uint16_t port = 161);

    /**
     * @brief Запускает основной цикл обработки SNMP-запросов.
     *
     * Метод блокирующий и выполняется до вызова stop()
     * или возникновения ошибки.
     *
     * @return true, если агент завершил работу без ошибок.
     */
    bool run();

    /**
     * @brief Указывает, запущен ли в данный момент агент.
     */
    bool isRunning() const;

    /**
     * @brief Запрашивает остановку SNMP-агента.
     *
     * Фактическая остановка происходит в основном цикле run().
     */
    void stop();

private:
    /**
     * @brief Обрабатывает входящий SNMP-пакет.
     */
    void processSnmpPacket(const uint8_t* data,
                           size_t size,
                           const sockaddr_in& clientAddr,
                           socklen_t clientLen);

    /**
     * @brief Отправляет SNMP-ответ клиенту.
     *
     * @return true в случае успешной отправки.
     */
    bool sendSnmpResponse(const uint8_t* data,
                          size_t size,
                          const sockaddr_in& clientAddr,
                          socklen_t clientLen);

private:
    int m_sock{ -1 };  ///< UDP-сокет агента.

    std::atomic<bool> m_running{ false };        ///< Агент находится в состоянии run().
    std::atomic<bool> m_stopRequested{ false };  ///< Запрошена остановка агента.

    UpsDataStore* m_store{ nullptr };  ///< Хранилище данных UPS (не владеет).
    SnmpCodec m_codec{};         ///< Кодек для SNMP-сообщений.
};

#endif  // SNMP_AGENT_H
