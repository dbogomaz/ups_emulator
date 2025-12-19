#include <gtest/gtest.h>

#include "ups_emulator.h"

class UpsEmulatorLifecycleTest : public ::testing::Test {
protected:
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_emulator";
    std::string data(const std::string& name) const { return dataDir + "/" + name; }
};

// ============================================================
// Part 1 — Preconditions
// ============================================================
// Тест 1.1: start() без выбранной модели невозможен
TEST_F(UpsEmulatorLifecycleTest, StartWithoutModelFails) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    ASSERT_TRUE(emulator.ok());
    EXPECT_FALSE(emulator.start());
    EXPECT_FALSE(emulator.isRunning());
}

// ============================================================
// Part 2 — Start / Stop lifecycle
// ============================================================
// Тест 2.1: Успешный start() и stop()
TEST_F(UpsEmulatorLifecycleTest, StartAndStopLifecycle) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    ASSERT_TRUE(emulator.ok()) << "Emulator initialization failed: " + emulator.lastError();
    ASSERT_TRUE(emulator.selectModel("APC")) << "Model selection failed: " + emulator.lastError();
    ASSERT_TRUE(emulator.start(1161)) << "Emulator start failed: " + emulator.lastError();
    EXPECT_TRUE(emulator.isRunning()) << "Emulator should be running after start()";
    emulator.stop();
    EXPECT_FALSE(emulator.isRunning()) << "Emulator should not be running after stop()";
}

// Тест 2.2: Повторный start() запрещён
TEST_F(UpsEmulatorLifecycleTest, DoubleStartFails) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    ASSERT_TRUE(emulator.ok()) << "Emulator initialization failed: " + emulator.lastError();
    ASSERT_TRUE(emulator.selectModel("APC")) << "Model selection failed: " + emulator.lastError();
    ASSERT_TRUE(emulator.start(1161)) << "Emulator start failed: " + emulator.lastError();
    EXPECT_FALSE(emulator.start(1161)) << "Second start() should fail";
    emulator.stop();
}

// Тест 2.3: При повторном вызове stop() состояние не меняется
TEST_F(UpsEmulatorLifecycleTest, StopIsIdempotent) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    ASSERT_TRUE(emulator.ok());
    ASSERT_TRUE(emulator.selectModel("APC"));
    ASSERT_TRUE(emulator.start(1161));
    emulator.stop();
    EXPECT_FALSE(emulator.isRunning());
    // повторный stop не должен ломать состояние
    emulator.stop();
    EXPECT_FALSE(emulator.isRunning());
}

// ============================================================
// Part 3 — Restrictions while running
// ============================================================
// Тест 3.1: Нельзя сменить модель при запущенном эмуляторе
TEST_F(UpsEmulatorLifecycleTest, SelectModelWhileRunningFails) {
    UpsEmulator emulator(data("valid_two_models.ini"));
    ASSERT_TRUE(emulator.ok());
    ASSERT_TRUE(emulator.selectModel("APC"));
    ASSERT_TRUE(emulator.start(1161));
    EXPECT_FALSE(emulator.selectModel("INELT"));
    EXPECT_EQ(emulator.currentModel(), "APC");
    emulator.stop();
}
