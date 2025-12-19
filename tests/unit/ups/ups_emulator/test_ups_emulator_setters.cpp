#include <gtest/gtest.h>

#include "ups_emulator.h"

class UpsEmulatorSettersTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_emulator = new UpsEmulator(data("valid_single_model.ini"));
        ASSERT_TRUE(m_emulator->ok());
        ASSERT_TRUE(m_emulator->selectModel("APC"));
        ASSERT_TRUE(m_emulator->start(m_testPort));
    }

    void TearDown() override {
        if (m_emulator) {
            m_emulator->stop();
            delete m_emulator;
            m_emulator = nullptr;
        }
    }

    void expectAllSettersSucceed() {
        EXPECT_TRUE(emulator().setInputVoltage(230));
        EXPECT_TRUE(emulator().setInputFrequency(50));
        EXPECT_TRUE(emulator().setChargeRemaining(80));
        EXPECT_TRUE(emulator().setBatteryTemperature(25));
        EXPECT_TRUE(emulator().setBatteryStatus("batteryStatus_APC_1"));
        EXPECT_TRUE(emulator().setOutputVoltage(220));
        EXPECT_TRUE(emulator().setOutputStatus("outputStatus_APC_1"));
    }

    void expectInvalidEnumSettersFail() {
        EXPECT_FALSE(emulator().setBatteryStatus("nonexistent"));
        EXPECT_FALSE(emulator().setOutputStatus("nonexistent"));
    }

    UpsEmulator& emulator() { return *m_emulator; }

    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_emulator";
    std::string data(const std::string& name) const { return dataDir + "/" + name; }

private:
    const uint16_t m_testPort{ 1161 };
    UpsEmulator* m_emulator{ nullptr };
};

//
// ============================================================
// Part 1 — Preconditions
// ============================================================
//

// Тест 1.1: Сеттеры недоступны без выбранной модели
TEST(UpsEmulatorSettersPreconditionsTest, SettersWithoutModelFail) {
    UpsEmulator emu(std::string(TEST_DATA_DIR) + "/ups_emulator/valid_single_model.ini");
    ASSERT_TRUE(emu.ok());
    EXPECT_FALSE(emu.setInputVoltage(230));
    EXPECT_FALSE(emu.setInputFrequency(50));
    EXPECT_FALSE(emu.setChargeRemaining(80));
    EXPECT_FALSE(emu.setBatteryTemperature(25));
    EXPECT_FALSE(emu.setBatteryStatus("batteryStatus_APC_1"));
    EXPECT_FALSE(emu.setOutputVoltage(220));
    EXPECT_FALSE(emu.setOutputStatus("outputStatus_APC_1"));
}

//
// ============================================================
// Part 2 —  Setters valid data
// ============================================================
//

// Тест 2.1: Сеттеры при запущенном эмуляторе
TEST_F(UpsEmulatorSettersTest, SettersWhileRunningSucceed) {
    expectAllSettersSucceed();
}

// Тест 2.2: Сеттеры доступны после stop()
TEST_F(UpsEmulatorSettersTest, SettersAfterStopSucceed) {
    emulator().stop();
    expectAllSettersSucceed();
}

//
// ============================================================
// Part 3 — Setters invalid data
// ============================================================
//

// Тест 3.1: Некорректные enum-значения отклоняются при запущенном эмуляторе
TEST_F(UpsEmulatorSettersTest, InvalidEnumValuesWhileRunningFail) {
    expectInvalidEnumSettersFail();
}

// Тест 3.2: Некорректные enum-значения отклоняются после stop()
TEST_F(UpsEmulatorSettersTest, InvalidEnumValuesAfterStopFail) {
    emulator().stop();
    expectInvalidEnumSettersFail();
}
