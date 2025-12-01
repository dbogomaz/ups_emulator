#include "ups_model_config.h"

#include <gtest/gtest.h>

const std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_model_config";

// Некорректный формат value-set (отсутствуют фигурные скобки)
TEST(UpsModelConfigTest, InvalidValueSet_NoBraces) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_valueset_nobraces.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("Invalid value-set"), std::string::npos)
        << cfg.lastError();
}

// Некорректный формат value-set (отсутствует закрывающая фигурная скобка)
TEST(UpsModelConfigTest, ValueSetMissingClosingBrace) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_valueset_missing_closing.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("brace"), std::string::npos) << cfg.lastError();
}

// Некорректный формат value-set (отсутствует запятая между записями)
TEST(UpsModelConfigTest, ValueSetMissingComma) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_valueset_missing_comma.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("entry"), std::string::npos) << cfg.lastError();
}

// Некорректный формат value-set (отсутствует двоеточие между ключом и значением)
TEST(UpsModelConfigTest, ValueSetMissingColon) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_valueset_missing_colon.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("Invalid value-set entry"), std::string::npos)
        << "Expected error about missing colon, got: " << cfg.lastError();
}
