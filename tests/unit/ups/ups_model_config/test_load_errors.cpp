#include <gtest/gtest.h>

#include "ups_model_config.h"

class UpsModelConfigErrorTest : public ::testing::Test {
protected:
    UpsModelConfig cfg;
    std::string dataDir = std::string(TEST_DATA_DIR) + "/ups_model_config";
    
    // Полный путь к файлу тестовых данных
    std::string data(const std::string& name) const { return dataDir + "/" + name; }

    // helper для проверки, что загрузка завершилась ошибкой
    bool loadExpectFail(const std::string& file, const std::string& section) {
        bool ok = cfg.load(file, section);
        EXPECT_FALSE(ok);
        EXPECT_FALSE(cfg.lastError().empty());
        return ok;
    }
};

// ---------------------------------------------------------------
// Ошибки файла и секций
// ---------------------------------------------------------------
// Файл не найден
TEST_F(UpsModelConfigErrorTest, FileNotFound) {
    loadExpectFail("no_such_file.ini", "APC");
    EXPECT_NE(cfg.lastError().find("Cannot open file"), std::string::npos);
}
// Отсутствующая секция
TEST_F(UpsModelConfigErrorTest, SectionNotFound) {
    loadExpectFail(std::string(TEST_DATA_DIR) + "/ups_models.ini", "NO_SUCH_SECTION");
    EXPECT_NE(cfg.lastError().find("not found"), std::string::npos);
}

// ---------------------------------------------------------------
// Ошибки обязательных полей
// ---------------------------------------------------------------
// Отсутствующее обязательное поле OID
TEST_F(UpsModelConfigErrorTest, MissingRequiredOID) {
    loadExpectFail(data("missing_oid.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("batteryStatusOID"), std::string::npos);
}
// Отсутствующее имя модели
TEST_F(UpsModelConfigErrorTest, MissingModelName) {
    loadExpectFail(data("no_modelname.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("modelName"), std::string::npos);
}
// Отсутствуют outputStatusValues
TEST_F(UpsModelConfigErrorTest, MissingOutputStatusValues) {
    loadExpectFail(data("no_outputStatus_values.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("outputStatusValues"), std::string::npos);
}
// Отсутствуют batteryStatusValues
TEST_F(UpsModelConfigErrorTest, MissingBatteryStatusValues) {
    loadExpectFail(data("no_batteryStatus_values.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("batteryStatusValues"), std::string::npos);
}

// ---------------------------------------------------------------
// Некорректные значения сложных полей (не формат value-set, а неверное число)
// ---------------------------------------------------------------
TEST_F(UpsModelConfigErrorTest, InvalidEnumValueBatteryStatus) {
    loadExpectFail(data("bad_enum_battery.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("Invalid integer"), std::string::npos);
}
TEST_F(UpsModelConfigErrorTest, InvalidEnumValueOutputStatus) {
    loadExpectFail(data("bad_enum_output.ini"), "APC");
    EXPECT_NE(cfg.lastError().find("Invalid integer"), std::string::npos);
}

// ---------------------------------------------------------------
// Ошибки строк формата key=value
// ---------------------------------------------------------------
// Параметр без знака равенства
TEST_F(UpsModelConfigErrorTest, LineWithoutEqualIsIgnored) {
    bool ok = cfg.load(data("no_equal.ini"), "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}
// Неизвестный ключ
TEST_F(UpsModelConfigErrorTest, UnknownKeyIsIgnored) {
    bool ok = cfg.load(data("unknown_key.ini"), "APC");
    EXPECT_TRUE(ok) << cfg.lastError();
}
