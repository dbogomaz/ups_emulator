#include <gtest/gtest.h>
#include "ups_model_config.h"

// Успешная загрузка секции APC
TEST(UpsModelConfigTest, LoadAPCSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "APC");

    // 1) Успешная загрузка
    ASSERT_TRUE(ok) << cfg.lastError();

    // 2) Проверяем, что имя модели загружено
    EXPECT_EQ(cfg.modelName(), "Smart-UPS RT 2000 XL");

    // 3) Проверяем OID
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());

    // 4) Проверяем значения байпаса
    ASSERT_EQ(cfg.bypassValues().size(), 3);
    EXPECT_EQ(cfg.bypassValues()[0], 6);
    EXPECT_EQ(cfg.bypassValues()[1], 9);
    EXPECT_EQ(cfg.bypassValues()[2], 10);
}

// Успешная загрузка секции INELT
TEST(UpsModelConfigTest, LoadINELTSuccess) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "INELT");

    // 1) Успешная загрузка
    ASSERT_TRUE(ok) << cfg.lastError();

    // 2) Проверяем имя модели
    EXPECT_EQ(cfg.modelName(), "MP3000RT");

    // 3) Проверяем OID
    EXPECT_FALSE(cfg.oids().modelNameOID.empty());

    // 4) Проверяем bypassValues
    ASSERT_EQ(cfg.bypassValues().size(), 1);
    EXPECT_EQ(cfg.bypassValues()[0], 4);
}

// Файл не найден
TEST(UpsModelConfigTest, FileNotFound) {
    UpsModelConfig cfg;
    bool ok = cfg.load("no_such_file.ini", "APC");

    // Ожидаем ошибку
    EXPECT_FALSE(ok);

    // lastError должен содержать текст
    EXPECT_FALSE(cfg.lastError().empty());

    // Проверка, что ошибка корректно сформирована
    EXPECT_NE(cfg.lastError().find("Cannot open file"), std::string::npos);
}

// Отсутствующая секция
TEST(UpsModelConfigTest, SectionNotFound) {
    UpsModelConfig cfg;
    bool ok = cfg.load("config/ups_models.ini", "NO_SUCH_SECTION");

    // Ожидаем ошибку
    EXPECT_FALSE(ok);

    // lastError должен быть заполнен
    EXPECT_FALSE(cfg.lastError().empty());

    // Проверяем, что ошибка касается отсутствующей секции
    EXPECT_NE(cfg.lastError().find("not found"), std::string::npos);
}

// Отсутствующее обязательное поле OID
TEST(UpsModelConfigTest, MissingRequiredOID) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/missing_oid.ini";
    bool ok = cfg.load(path, "APC");

    // Ожидаем ошибку
    EXPECT_FALSE(ok);

    // lastError должен быть заполнен
    EXPECT_FALSE(cfg.lastError().empty());

    // Проверяем, что сообщение относится к отсутствию обязательного поля
    EXPECT_NE(cfg.lastError().find("batteryStatusOID"), std::string::npos) << cfg.lastError();
}

// Некорректные значения bypassStatusAllowed
TEST(UpsModelConfigTest, InvalidBypassValues) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/bad_bypass.ini";
    bool ok = cfg.load(path, "APC");

    // Должен вернуть false
    EXPECT_FALSE(ok);

    // lastError должен содержать информацию о некорректном числе
    EXPECT_NE(cfg.lastError().find("Invalid integer"), std::string::npos) << cfg.lastError();
    EXPECT_NE(cfg.lastError().find("aa"), std::string::npos) << cfg.lastError();
}

// Параметр без знака равенства
TEST(UpsModelConfigTest, LineWithoutEqualIsIgnored) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/no_equal.ini";

    bool ok = cfg.load(path, "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}

// Неизвестный ключ
TEST(UpsModelConfigTest, UnknownKeyIsIgnored) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/unknown_key.ini";

    bool ok = cfg.load(path, "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}

// Отсутствующее имя модели
TEST(UpsModelConfigTest, MissingModelName) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/no_modelname.ini";

    bool ok = cfg.load(path, "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("modelName"), std::string::npos);
}

// Отсутствующее значение bypassStatusAllowed
TEST(UpsModelConfigTest, MissingBypassValues) {
    UpsModelConfig cfg;
    std::string path = std::string(TEST_DATA_DIR) + "/no_bypass_allowed.ini";

    bool ok = cfg.load(path, "APC");
    EXPECT_FALSE(ok);
    EXPECT_NE(cfg.lastError().find("bypassStatusAllowed"), std::string::npos);
}
