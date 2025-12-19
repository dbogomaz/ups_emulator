#include "snmp_agent.h"

#include <arpa/inet.h>   // htons(), ntohs(), inet_ntoa()
#include <poll.h>        // poll(), struct pollfd
#include <sys/socket.h>  // socket(), bind(), recvfrom(), sendto()
#include <unistd.h>      // close()

#include <cerrno>  // errno, EINTR
#include <cstdio>  // printf(), perror()

using namespace snmp;

SnmpAgent::SnmpAgent(UpsDataStore* store) : m_store(store) {}

bool SnmpAgent::bind(uint16_t port) {
    // Защита от повтроного вызова
    if (m_sock >= 0) {
        printf("SnmpAgent::bind(): already bound\n");
        return false;
    }

    // 1) Создаем UDP сокет
    m_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    // AF_INET — IPv4 (адрес 32 бита: 192.168.1.10) какой тип адресов будет использовать сокет
    // SOCK_DGRAM - UDP-сокет тип сокета
    // 0 - протокол если SOCK_DGRAM, ядро автоматически использует протокол UDP.
    // 0 - ядро выберет подходящий протокол по умолчанию
    if (m_sock < 0) {
        perror("socket");  // LCOV_EXCL_LINE
        return false;      // LCOV_EXCL_LINE
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
        // LCOV_EXCL_START
        perror("setsockopt");
        ::close(m_sock);
        m_sock = -1;
        return false;
        // LCOV_EXCL_STOP
    }

    // 3) Привязываем сокет к порту
    struct sockaddr_in addr{};                 // адрес куда принимаем пакеты
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

    // printf("SnmpAgent successfully bound to UDP port %u\n", port);
    return true;
}

bool SnmpAgent::run() {
    // Проверка готовности
    if (m_sock < 0) {
        printf("SnmpAgent::run(): socket not initialized\n");
        return false;
    }

    // Защита от повторного запуска
    if (m_running.load()) {
        printf("SnmpAgent::run(): already running\n");
        return false;
    }

    m_stopRequested.store(false);
    m_running.store(true);

    printf("SnmpAgent running...\n");

    // Максимальный размер SNMP пакета - хватит 4096
    uint8_t buffer[4096];

    while (!m_stopRequested.load()) {
        struct pollfd pfd;
        pfd.fd = m_sock;
        pfd.events = POLLIN;  // ожидаем готовность сокета к чтению
        pfd.revents = 0;      // заполняется ядром, обнулим перед этим
        // revents битовое поле POLLIN — можно читать
        //                      POLLERR   — ошибка
        //                      POLLHUP   — hangup
        //                      POLLNVAL  — fd невалиден

        // Таймаут 500 мс
        int rc = ::poll(&pfd, 1, 500);

        if (rc < 0) {
            // LCOV_EXCL_START
            if (errno == EINTR) {
                continue;
            }
            perror("poll");
            break;
            // LCOV_EXCL_STOP
        }

        // за timeout событий не было — просто проверяем stopRequested
        if (rc == 0) continue;

        // LCOV_EXCL_START
        // следующий участок кода исключен из покрытия тестами, так как
        // он зависит от внешних событий (пакетов в сеть)
        // писать такие тесты не буду да и не хочется разбираться с этим

        // Ошибка или закрытие сокета — выходим
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) break;
        
        // если сокет не готов к чтению — не читаем
        if (!(pfd.revents & POLLIN)) continue;
        
        struct sockaddr_in clientAddr{};

        // Чтение UDP пакета
        socklen_t clientLen = sizeof(clientAddr);
        // clang-format off
        ssize_t received = recvfrom(
            m_sock, 
            buffer, 
            sizeof(buffer), 
            0, // блокирующий recvfrom (не блокируется из-за предварительного poll)
            (struct sockaddr*)&clientAddr, 
            &clientLen
        );
        // clang-format on

        // ---- обработка ошибок recvfrom ----
        if (received < 0) {
            // Остановка по запросу или прерывание
            if (m_stopRequested.load()) {
                break;
            }
            // Прерывание сигналом — не ошибка
            if (errno == EINTR) {
                continue;
            }
            // Реальная ошибка сокета
            perror("recvfrom");
            break;
        }

        // Обработка пакета
        processSnmpPacket(buffer, (size_t)received, clientAddr, clientLen);

        // LCOV_EXCL_STOP
    }

    // Останавливаемся и закрываем сокет

    ::close(m_sock);
    m_sock = -1;
    m_running.store(false);
    printf("SnmpAgent stopped\n");
    return true;
}

void SnmpAgent::stop() {
    // stop() имеет смысл только для запущенного агента
    if (!m_running.load()) {
        return;
    }

    m_stopRequested.store(true);

    if (m_sock >= 0) {
        ::shutdown(m_sock, SHUT_RDWR);
    }
}


// LCOV_EXCL_START
// Методы обработки SNMP-пакета и отправки ответа
// зависят от сетевого I/O и покрываются только интеграционными тестами.
// В unit-тестах SnmpAgent исключены из покрытия осознанно и в трезвом уме.

void SnmpAgent::processSnmpPacket(const uint8_t* data,
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
    sendSnmpResponse(response.data(), response.size(), clientAddr, clientLen);
}

bool SnmpAgent::sendSnmpResponse(const uint8_t* data,
                                 size_t size,
                                 const sockaddr_in& clientAddr,
                                 socklen_t clientLen) {
    ssize_t sent = sendto(m_sock, data, size, 0, (const struct sockaddr*)&clientAddr, clientLen);

    if (sent < 0) {
        perror("sendto");
        return false;
    }

    return true;
}

// LCOV_EXCL_STOP

bool SnmpAgent::isRunning() const { return m_running.load(); }
