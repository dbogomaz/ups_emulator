#include <gtest/gtest.h>

#include "ups_model_config.h"

class UpsModelConfigSuccessTest : public ::testing::Test {
protected:
    UpsModelConfig cfg;
    std::string iniFile = "config/ups_models.ini";

    // helper для удобной загрузки секции
    bool loadSection(const std::string& section) {
        bool ok = cfg.load(iniFile, section);
        EXPECT_TRUE(ok) << cfg.lastError();
        if (!ok) return false;
        // Проверяем, что базовые OID присутствуют
        EXPECT_FALSE(cfg.oids().modelNameOID.empty()) << cfg.lastError();
        return true;
    }

    // Получить batteryStatusValues с проверками
    const std::map<std::string, int>& batteryValues() {
        const auto& bs = cfg.definedFields().batteryStatusSet.nameToValue;
        EXPECT_FALSE(bs.empty()) << cfg.lastError();
        return bs;
    }

    // Получить outputStatusValues с проверками
    const std::map<std::string, int>& outputValues() {
        const auto& os = cfg.definedFields().outputStatusSet.nameToValue;
        EXPECT_FALSE(os.empty()) << cfg.lastError();
        return os;
    }
};

// ---------------------------------------------------------------
// Успешная загрузка секции APC
// ---------------------------------------------------------------
TEST_F(UpsModelConfigSuccessTest, LoadAPCSuccess) {
    ASSERT_TRUE(loadSection("APC"));
    EXPECT_EQ(cfg.modelName(), "Smart-UPS RT 2000 XL");
    // batteryStatusValues
    const auto& bs = batteryValues();
    EXPECT_EQ(bs.at("batteryNormal"), 2);
    EXPECT_EQ(bs.at("batteryLow"), 3);
    // outputStatusValues
    const auto& os = outputValues();
    EXPECT_EQ(os.at("onLine"), 2);
}

// ---------------------------------------------------------------
// Успешная загрузка секции INELT
// ---------------------------------------------------------------
TEST_F(UpsModelConfigSuccessTest, LoadINELTSuccess) {
    ASSERT_TRUE(loadSection("INELT"));
    EXPECT_EQ(cfg.modelName(), "MP3000RT");
    // batteryStatusValues
    const auto& bs = batteryValues();
    EXPECT_EQ(bs.at("batteryNormal"), 2);
    EXPECT_NE(bs.find("batteryDepleted"), bs.end());
    // outputStatusValues
    const auto& os = outputValues();
    EXPECT_EQ(os.at("normal"), 3);
}
