#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "snmp_agent.h"
#include "ups_data_store.h"

// ------------------------------------------------------------
// SnmpAgentTest fixture
// ------------------------------------------------------------
class SnmpAgentTest : public ::testing::Test {
protected:
    UpsDataStore store;
    SnmpAgent agent{ &store };
    const uint16_t testPort = 1161;
    void waitUntilRunning(std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) {
        const auto start = std::chrono::steady_clock::now();
        while (!agent.isRunning()) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                FAIL() << "Timeout waiting for SnmpAgent to enter running state";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
};

#if 1  // Part 1 — Initial state
// ============================================================
// Part 1 — Initial state
// ============================================================

// Тест 1.1: Агент сразу после создания не запущен
TEST_F(SnmpAgentTest, Initial_1_NotRunning) { EXPECT_FALSE(agent.isRunning()); }

// Тест 1.2: Запуск без bind() невозможен
TEST_F(SnmpAgentTest, Initial_2_RunWithoutBindFails) {
    EXPECT_FALSE(agent.run());
    EXPECT_FALSE(agent.isRunning());
}

#endif

#if 1  // Part 2 — Bind
// ============================================================
// Part 2 — Bind
// ============================================================

// Тест 2.1: bind() успешно выполняется на свободном порту
TEST_F(SnmpAgentTest, Bind_1_SucceedsOnFreePort) {
    ASSERT_TRUE(agent.bind(testPort));
    EXPECT_FALSE(agent.isRunning());
}

// Тест 2.2: Повторный вызов bind() запрещён
TEST_F(SnmpAgentTest, Bind_2_FailsOnSecondCall) {
    ASSERT_TRUE(agent.bind(testPort));
    EXPECT_FALSE(agent.bind(testPort));
}

// Тест 2.3: bind() не переводит агент в состояние running
TEST_F(SnmpAgentTest, Bind_3_DoesNotStartAgent) {
    ASSERT_TRUE(agent.bind(testPort));
    EXPECT_FALSE(agent.isRunning());
}

#endif

#if 1  // Part 3 — Run lifecycle
// ============================================================
// Part 3 — Run lifecycle
// ============================================================

// Тест 3.1: run() после bind() переводит агент в состояние running
TEST_F(SnmpAgentTest, Run_1_StartsAfterBind) {
    ASSERT_TRUE(agent.bind(testPort));
    std::thread t([this]() { agent.run(); });
    waitUntilRunning();
    EXPECT_TRUE(agent.isRunning());
    agent.stop();
    t.join();
    EXPECT_FALSE(agent.isRunning());
}

// Тест 3.2: stop() корректно останавливает работающий агент
TEST_F(SnmpAgentTest, Run_2_StopStopsAgent) {
    ASSERT_TRUE(agent.bind(testPort));
    std::thread t([this]() { agent.run(); });
    waitUntilRunning();
    ASSERT_TRUE(agent.isRunning());
    agent.stop();
    t.join();
    EXPECT_FALSE(agent.isRunning());
}

// Тест 3.3: Повторный запуск после stop() возможен при повторном bind()
TEST_F(SnmpAgentTest, Run_3_RunStopRunWithRebindIsAllowed) {
    ASSERT_TRUE(agent.bind(testPort));
    // Первый запуск
    std::thread t1([this]() { agent.run(); });
    waitUntilRunning();
    ASSERT_TRUE(agent.isRunning());
    agent.stop();
    t1.join();
    EXPECT_FALSE(agent.isRunning());
    // Повторный bind - важен при текущем API
    ASSERT_TRUE(agent.bind(testPort));
    // Второй запуск
    std::thread t2([this]() { agent.run(); });
    waitUntilRunning();
    EXPECT_TRUE(agent.isRunning());
    agent.stop();
    t2.join();
    EXPECT_FALSE(agent.isRunning());
}

#endif

#if 1  // Part 4 — Misuse / protection
// ============================================================
// Part 4 — Misuse / protection
// ============================================================

// Тест 4.1: Повторный вызов run() при уже работающем агенте запрещён
TEST_F(SnmpAgentTest, Misuse_1_RunWhileAlreadyRunningFails) {
    ASSERT_TRUE(agent.bind(testPort));
    std::thread t([this]() { agent.run(); });
    waitUntilRunning();
    ASSERT_TRUE(agent.isRunning());
    // Повторный run() должен быть отклонён
    EXPECT_FALSE(agent.run());
    EXPECT_TRUE(agent.isRunning());
    agent.stop();
    t.join();
    EXPECT_FALSE(agent.isRunning());
}

// Тест 4.2: Вызов stop() до run() безопасен
TEST_F(SnmpAgentTest, Misuse_2_StopBeforeRunIsSafe) {
    ASSERT_TRUE(agent.bind(testPort)) << "bind() failed";
    // stop() до run() не должен ломать состояние
    agent.stop();
    EXPECT_FALSE(agent.isRunning()) << "Agent should not be running after stop() before run()";
    // После этого run() всё ещё возможен
    std::thread t([this]() { agent.run(); });
    waitUntilRunning();
    EXPECT_TRUE(agent.isRunning()) << "Agent should be running after run()";
    agent.stop();
    t.join();
    EXPECT_FALSE(agent.isRunning()) << "Agent should not be running after stop()";
}

// Тест 4.3: Повторный вызов stop() безопасен
TEST_F(SnmpAgentTest, Misuse_3_StopMultipleTimesIsSafe) {
    ASSERT_TRUE(agent.bind(testPort));
    std::thread t([this]() { agent.run(); });
    waitUntilRunning();
    ASSERT_TRUE(agent.isRunning());
    // Многократные stop()
    agent.stop();
    agent.stop();
    agent.stop();
    t.join();
    EXPECT_FALSE(agent.isRunning());
}

#endif
