#include <gtest/gtest.h>

#include "ups_model_config.h"


// ---------------------------------------------------------------
// Успешная загрузка секции APC
// ---------------------------------------------------------------
TEST(UpsModelConfigTest, LoadAPCSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "APC");
    ASSERT_TRUE(ok) << cfg.lastError();
    EXPECT_EQ(cfg.modelName(), "Smart-UPS RT 2000 XL");
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());

    // batteryStatusValues
    const auto& bs = cfg.definedFields().batteryStatusSet.nameToValue;
    ASSERT_FALSE(bs.empty()) << cfg.lastError();
    EXPECT_EQ(bs.at("batteryNormal"), 2);
    EXPECT_EQ(bs.at("batteryLow"), 3);

    // outputStatusValues
    const auto& os = cfg.definedFields().outputStatusSet.nameToValue;
    ASSERT_FALSE(os.empty()) << cfg.lastError();
    EXPECT_EQ(os.at("onLine"), 2);
}


// ---------------------------------------------------------------
// Успешная загрузка секции INELT
// ---------------------------------------------------------------
TEST(UpsModelConfigTest, LoadINELTSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "INELT");
    ASSERT_TRUE(ok) << cfg.lastError();
    EXPECT_EQ(cfg.modelName(), "MP3000RT");
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());

    // batteryStatusValues
    const auto& bs = cfg.definedFields().batteryStatusSet.nameToValue;
    ASSERT_FALSE(bs.empty()) << cfg.lastError();
    EXPECT_EQ(bs.at("batteryNormal"), 2);
    EXPECT_NE(bs.find("batteryDepleted"), bs.end());

    // outputStatusValues
    const auto& os = cfg.definedFields().outputStatusSet.nameToValue;
    ASSERT_FALSE(os.empty()) << cfg.lastError();
    EXPECT_EQ(os.at("normal"), 3);
}
