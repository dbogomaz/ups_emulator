#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "snmp_agent.h"
#include "ups_data_store.h"

// ------------------------------------------------------------
// Заглушка UpsDataStore
// ------------------------------------------------------------
class DummyUpsDataStore : public UpsDataStore {
    // Ничего не нужно: в этом тесте пакеты не обрабатываются
};

// ------------------------------------------------------------
// SnmpAgent lifecycle test
// ------------------------------------------------------------
TEST(SnmpAgentTest, BindRunStopLifecycle) {
    DummyUpsDataStore store;
    SnmpAgent agent(&store);
    // Используем нестандартный порт, чтобы не требовать root
    const uint16_t port = 1161;
    ASSERT_TRUE(agent.bind(port));
    ASSERT_FALSE(agent.isRunning());
    // Запуск агента в отдельном потоке
    std::thread agentThread([&agent]() { agent.run(); });
    // Даём агенту войти в recvfrom()
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(agent.isRunning());
    // Останавливаем агент
    agent.stop();
    agentThread.join();
    EXPECT_FALSE(agent.isRunning());
}

// TEST(SnmpAgentTest, DoubleBindFails)
// {
//     DummyUpsDataStore store;
//     SnmpAgent agent(&store);

//     ASSERT_TRUE(agent.bind(1162));
//     EXPECT_FALSE(agent.bind(1162));
// }

