#ifndef SNMP_AGENT_H
#define SNMP_AGENT_H

#include <netinet/in.h>

#include <cstddef>
#include <cstdint>

#include "snmp_codec.h"
#include "ups_data_store.h"

class SnmpAgent {
public:
    explicit SnmpAgent(UpsDataStore* store);

    // Инициализация UDP сокета
    bool bind(uint16_t port = 161);

    // Основной цикл обработки
    bool run();

    // агент запущен
    bool isRunning() const;

    // Остановка агента
    void stop();

private:
    // Обработка пакета
    void processSnmpPacket(const uint8_t* data,
                           size_t size,
                           const sockaddr_in& clientAddr,
                           socklen_t clientLen);

    // Отправка ответа клиенту
    bool sendSnmpResponse(const uint8_t* data,
                          size_t size,
                          const sockaddr_in& clientAddr,
                          socklen_t clientLen);

private:
    int m_sock{ -1 };
    bool m_running{ false };        // run() сейчас выполняется
    bool m_stopRequested{ false };  // запрошена остановка

    UpsDataStore* m_store{ nullptr };
    snmp::SnmpCodec m_codec{};
};

#endif
