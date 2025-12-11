#include "snmp_agent.h"

#include <arpa/inet.h>  // htons(), inet_ntoa()
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>  // socket(), bind(), recvfrom(), sendto()
#include <unistd.h>      // close()

using namespace snmp;

SnmpAgent::SnmpAgent(UpsDataStore* store) : m_store(store) {}

bool SnmpAgent::bind(uint16_t port) {
    // 1) Создаем UDP сокет
    m_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    // AF_INET — IPv4 (адрес 32 бита: 192.168.1.10) какой тип адресов будет использовать сокет
    // SOCK_DGRAM - UDP-сокет тип сокета
    // 0 - протокол если SOCK_DGRAM, ядро автоматически использует протокол UDP.
    // 0 - ядро выберет подходящий протокол по умолчанию
    if (m_sock < 0) {
        perror("socket");
        return false;
    }
    // в результате m_sock - это идентификатор сокета т.е. дескриптор, ассоциированный с UDP-сокетом

    // 2) Позволяем повторно использовать порт (полезно при перезапуске приложения)
    // setsockopt - системный вызов, который устанавливает опцию сокета.
    // m_sock  — дескриптор UDP-сокета, который мы только что создали.
    // SOL_SOCKET — уровень опции: означает, что опция относится к самому сокету (а не, например, к
    // протоколу TCP). SO_REUSEADDR — опция, позволяющая повторно использовать адрес/порт. Для UDP
    // (SNMP особенно) обязательная опция. &opt — указатель на значение (целое число). sizeof(opt) —
    // размер этого значения. opt = 1 означает: включить опцию.
    int opt = 1;
    if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        ::close(m_sock);
        m_sock = -1;
        return false;
    }

    // 3) Привязываем сокет к порту
    struct sockaddr_in addr;                   // адрес куда принимаем пакеты
    memset(&addr, 0, sizeof(addr));            // обнуляем
    addr.sin_family = AF_INET;                 // тип адресов IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // принимать пакеты на всех локальных интерфейсах.
    //  addr.sin_addr.s_addr = inet_addr("192.168.4.1");
    addr.sin_port = htons(port);  // принимаем пакеты с порта port
    // собственно привязываем сокет к порту адреса
    if (::bind(m_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        ::close(m_sock);
        m_sock = -1;
        return false;
    }

    m_running = true;  // TODO: название флага не корректное
    printf("SnmpAgent successfully bound to UDP port %u\n", port);
    return true;
}

void SnmpAgent::run() {
    if (m_sock < 0) {
        printf("SnmpAgent::run(): socket not initialized\n");
        return;
    }

    printf("SnmpAgent running...\n");

    // Максимальный размер SNMP пакета - хватит 4096
    uint8_t buffer[4096];

    while (m_running) {
        struct sockaddr_in clientAddr;
        memset(&clientAddr, 0, sizeof(clientAddr));

        // Блокирующее чтение UDP пакета
        socklen_t clientLen = sizeof(clientAddr);
        // clang-format off
        ssize_t received = recvfrom(
            m_sock, 
            buffer, 
            sizeof(buffer), 
            0, 
            (struct sockaddr*)&clientAddr, 
            &clientLen
        );
        // clang-format on

        if (!m_running) {
            break;  // нас остановили извне
        }

        if (received < 0) {
            perror("recvfrom");
            continue;
        }

        // Отладочная печать (можно убрать позже)
        printf("Received %ld bytes from %s:%d\n",
               (long)received,
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        // Пока работаем в синхронном режиме
        processPacketSync(buffer, (size_t)received, clientAddr, clientLen);
    }

    printf("SnmpAgent stopped\n");
}

void SnmpAgent::stop() {
    m_running = false;

    if (m_sock >= 0) {
        ::close(m_sock);
        m_sock = -1;
    }
}

void SnmpAgent::processPacketSync(const uint8_t* data,
                                  size_t size,
                                  const sockaddr_in& clientAddr,
                                  socklen_t clientLen) {
    snmp::SnmpGetRequest req;
    std::string err;

    // 1. Декодирование запроса
    if (!m_codec.decodeGetRequest(data, size, req, &err)) {
        printf("SNMP decode error: %s\n", err.c_str());
        return;  // пока просто игнорируем неверные запросы
    }

    // 2. Формирование ответа
    std::vector<uint8_t> response;
    if (!m_codec.encodeGetResponse(req, *m_store, response)) {
        printf("SNMP encode error\n");
        return;
    }

    // 3. Отправка клиенту
    sendResponse(response.data(), response.size(), clientAddr, clientLen);
}

bool SnmpAgent::sendResponse(const uint8_t* data,
                             size_t size,
                             const sockaddr_in& clientAddr,
                             socklen_t clientLen) {
    ssize_t sent = sendto(m_sock, data, size, 0, (const struct sockaddr*)&clientAddr, clientLen);

    if (sent < 0) {
        perror("sendto");
        return false;
    }

    printf("Sent %ld bytes to %s:%d\n",
           (long)sent,
           inet_ntoa(clientAddr.sin_addr),
           ntohs(clientAddr.sin_port));

    return true;
}
