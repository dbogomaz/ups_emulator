#include <gtest/gtest.h>

#include "ups_emulator.h"

class UpsEmulatorInitTest : public ::testing::Test {
protected:
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_emulator";
    std::string data(const std::string& name) const { return dataDir + "/" + name; }
};

// ============================================================
// Part 1 — Initial state
// ============================================================

// Тест 1.1: Успешная инициализация с корректным конфигом
TEST_F(UpsEmulatorInitTest, ConstructWithValidConfig) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    EXPECT_TRUE(emulator.ok());
    EXPECT_FALSE(emulator.isRunning());
    const auto& models = emulator.availableModels();
    ASSERT_EQ(models.size(), 1u);
    EXPECT_EQ(models[0], "APC");
    EXPECT_TRUE(emulator.currentModel().empty());
}

// Тест 1.2: Инициализация с отсутствующим конфигом завершается ошибкой
TEST_F(UpsEmulatorInitTest, ConstructWithMissingConfigFails) {
    UpsEmulator emulator(data("does_not_exist.ini"));
    EXPECT_FALSE(emulator.ok());
    EXPECT_FALSE(emulator.isRunning());
    EXPECT_FALSE(emulator.lastError().empty());
    EXPECT_TRUE(emulator.availableModels().empty());
}

// Тест 1.3: Инициализация с битым конфигом завершается ошибкой
TEST_F(UpsEmulatorInitTest, ConstructWithBrokenConfigFails) {
    UpsEmulator emulator(data("invalid_broken_config.ini"));
    EXPECT_FALSE(emulator.ok());
    EXPECT_FALSE(emulator.lastError().empty());
    EXPECT_TRUE(emulator.availableModels().empty());
}

// Тест 1.4: Состояние ok() стабильно и не изменяется при повторных вызовах
TEST_F(UpsEmulatorInitTest, OkStateIsStable) {
    UpsEmulator emulator(data("valid_single_model.ini"));
    ASSERT_TRUE(emulator.ok());
    EXPECT_TRUE(emulator.ok());
    EXPECT_TRUE(emulator.ok());
}
