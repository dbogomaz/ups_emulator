#include <gtest/gtest.h>

#include <algorithm>

#include "ups_emulator.h"

class UpsEmulatorModelSelectionTest : public ::testing::Test {
protected:
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_emulator";
    std::string data(const std::string& name) const { return dataDir + "/" + name; }
};

// ============================================================
// Part 1 — Initial state
// ============================================================

// Тест 1.1: После инициализации модель не выбрана
TEST_F(UpsEmulatorModelSelectionTest, NoModelSelectedInitially) {
    UpsEmulator emulator(data("valid_two_models.ini"));
    EXPECT_TRUE(emulator.ok());
    EXPECT_TRUE(emulator.currentModel().empty());
}

// ============================================================
// Part 2 — Model selection
// ============================================================

// Тест 2.1: Выбор существующей модели APC
TEST_F(UpsEmulatorModelSelectionTest, SelectExistingModelApc) {
    UpsEmulator emulator(data("valid_two_models.ini"));
    ASSERT_TRUE(emulator.ok()) << emulator.lastError();
    ASSERT_TRUE(emulator.selectModel("APC")) << "selectModel failed: " << emulator.lastError();
    EXPECT_EQ(emulator.currentModel(), "APC");
    const auto& battery = emulator.availableBatteryStatuses();
    const auto& output = emulator.availableOutputStatuses();
    ASSERT_EQ(battery.size(), 3u);
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_APC_1"), battery.end());
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_APC_2"), battery.end());
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_APC_3"), battery.end());
    ASSERT_EQ(output.size(), 3u);
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_APC_1"), output.end());
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_APC_2"), output.end());
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_APC_3"), output.end());
}

// Тест 2.2: Переключение с APC на INELT
TEST_F(UpsEmulatorModelSelectionTest, SwitchModelFromApcToInelt) {
    UpsEmulator emulator(data("valid_two_models.ini"));
    ASSERT_TRUE(emulator.ok());
    ASSERT_TRUE(emulator.selectModel("APC"));
    ASSERT_TRUE(emulator.selectModel("INELT"));
    EXPECT_EQ(emulator.currentModel(), "INELT");
    const auto& battery = emulator.availableBatteryStatuses();
    const auto& output = emulator.availableOutputStatuses();
    ASSERT_EQ(battery.size(), 4u);
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_INELT_1"), battery.end());
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_INELT_2"), battery.end());
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_INELT_3"), battery.end());
    EXPECT_NE(std::find(battery.begin(), battery.end(), "batteryStatus_INELT_4"), battery.end());
    ASSERT_EQ(output.size(), 4u);
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_INELT_1"), output.end());
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_INELT_2"), output.end());
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_INELT_3"), output.end());
    EXPECT_NE(std::find(output.begin(), output.end(), "outputStatus_INELT_4"), output.end());
}

// Тест 2.3: Выбор несуществующей модели завершается ошибкой
TEST_F(UpsEmulatorModelSelectionTest, SelectNonExistingModelFails) {
    UpsEmulator emulator(data("valid_two_models.ini"));
    ASSERT_TRUE(emulator.ok());
    EXPECT_FALSE(emulator.selectModel("NON_EXISTENT"));
    EXPECT_TRUE(emulator.currentModel().empty());
}
