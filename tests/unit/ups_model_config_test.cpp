#include <gtest/gtest.h>
#include "ups_model_config.h"

const std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_model_config";

// Успешная загрузка секции APC
TEST(UpsModelConfigTest, LoadAPCSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "APC");
    ASSERT_TRUE(ok) << cfg.lastError();
    EXPECT_EQ(cfg.modelName(), "Smart-UPS RT 2000 XL");
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());
    // batteryStatusValues
    const auto& bs = cfg.definedFields().batteryStatusSet.nameToValue;
    ASSERT_FALSE(bs.empty());
    EXPECT_EQ(bs.at("batteryNormal"), 2);
    EXPECT_EQ(bs.at("batteryLow"), 3);
    // outputStatusValues
    const auto& os = cfg.definedFields().outputStatusSet.nameToValue;
    ASSERT_FALSE(os.empty());
    EXPECT_EQ(os.at("onLine"), 2);
}

// Успешная загрузка секции INELT
TEST(UpsModelConfigTest, LoadINELTSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "INELT");
    ASSERT_TRUE(ok) << cfg.lastError();
    EXPECT_EQ(cfg.modelName(), "MP3000RT");
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());
    const auto& bs = cfg.definedFields().batteryStatusSet.nameToValue;
    ASSERT_FALSE(bs.empty());
    EXPECT_NE(bs.find("batteryDepleted"), bs.end());
}

// Файл не найден
TEST(UpsModelConfigTest, FileNotFound) {
    UpsModelConfig cfg;
    bool ok = cfg.load("no_such_file.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(cfg.lastError().empty());
    // Проверка, что ошибка корректно сформирована
    EXPECT_NE(cfg.lastError().find("Cannot open file"), std::string::npos);
}

// Отсутствующая секция
TEST(UpsModelConfigTest, SectionNotFound) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "NO_SUCH_SECTION");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(cfg.lastError().empty());
    // Проверяем, что ошибка касается отсутствующей секции
    EXPECT_NE(cfg.lastError().find("not found"), std::string::npos);
}

// Отсутствующее обязательное поле OID
TEST(UpsModelConfigTest, MissingRequiredOID) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/missing_oid.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(cfg.lastError().empty());
    // Проверяем, что сообщение относится к отсутствию обязательного поля
    EXPECT_NE(cfg.lastError().find("batteryStatusOID"), std::string::npos) << cfg.lastError();
}

// Некорректное значение в перечислении batteryStatusValues
TEST(UpsModelConfigTest, InvalidEnumValueBatteryStatus) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_enum_battery.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_FALSE(cfg.lastError().empty());
    EXPECT_NE(cfg.lastError().find("Invalid integer"), std::string::npos);
}

// Некорректное значение в перечислении outputStatusValues
TEST(UpsModelConfigTest, InvalidEnumValueOutputStatus) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_enum_output.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("Invalid integer"), std::string::npos);
}


// Параметр без знака равенства
TEST(UpsModelConfigTest, LineWithoutEqualIsIgnored) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/no_equal.ini", "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}

// Неизвестный ключ
TEST(UpsModelConfigTest, UnknownKeyIsIgnored) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/unknown_key.ini", "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}

// Отсутствующее имя модели
TEST(UpsModelConfigTest, MissingModelName) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/no_modelname.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("modelName"), std::string::npos);
}

// Отсутствуют outputStatusValues
TEST(UpsModelConfigTest, MissingOutputStatusValues) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/no_outputStatus_values.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("outputStatusValues"), std::string::npos);
}

// Отсутствуют batteryStatusValues
TEST(UpsModelConfigTest, MissingBatteryStatusValues) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/no_batteryStatus_values.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("batteryStatusValues"), std::string::npos);
}

TEST(UpsModelConfigTest, InvalidValueSet_NoBraces) {
    UpsModelConfig cfg;
    bool ok = cfg.load(dataDir + "/bad_valueset_nobraces.ini", "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("Invalid value-set format"), std::string::npos);
}
