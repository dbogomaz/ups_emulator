#include <gtest/gtest.h>

#include "ups_model_config.h"

class UpsModelConfigValueSetErrorTest : public ::testing::Test {
protected:
    UpsModelConfig cfg;
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_model_config";

    std::string data(const std::string& name) const { return dataDir + "/" + name; }

    bool loadExpectFail(const std::string& file) {
        bool ok = cfg.load(file, "APC");
        EXPECT_FALSE(ok);
        EXPECT_FALSE(cfg.lastError().empty());
        return ok;
    }
};

// ---------------------------------------------------------------
// Ошибки формата value-set
// ---------------------------------------------------------------
// Отсутствуют фигурные скобки
TEST_F(UpsModelConfigValueSetErrorTest, NoBraces) {
    loadExpectFail(data("bad_valueset_nobraces.ini"));
    EXPECT_NE(cfg.lastError().find("Invalid value-set"), std::string::npos) << cfg.lastError();
}
// Отсутствует закрывающая фигурная скобка
TEST_F(UpsModelConfigValueSetErrorTest, MissingClosingBrace) {
    loadExpectFail(data("bad_valueset_missing_closing.ini"));
    EXPECT_NE(cfg.lastError().find("brace"), std::string::npos) << cfg.lastError();
}
// Отсутствует запятая между записями
TEST_F(UpsModelConfigValueSetErrorTest, MissingComma) {
    loadExpectFail(data("bad_valueset_missing_comma.ini"));
    EXPECT_NE(cfg.lastError().find("entry"), std::string::npos) << cfg.lastError();
}
// Отсутствует двоеточие между ключом и значением
TEST_F(UpsModelConfigValueSetErrorTest, MissingColon) {
    loadExpectFail(data("bad_valueset_missing_colon.ini"));
    EXPECT_NE(cfg.lastError().find("Invalid value-set entry"), std::string::npos)
        << cfg.lastError();
}
