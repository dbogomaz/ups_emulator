#include "ups_data_store.h"

#include <gtest/gtest.h>

#include "ups_model_config.h"
#include "ups_types.h"

class UpsDataStoreTest : public ::testing::Test {
protected:
    UpsModelConfig cfg;
    UpsDataStore store;
    ErrorMessage err;

    // INI-файл для тестов
    std::string iniPath = "config/ups_models.ini";
    IniSectionName section = "APC";

    void SetUp() override {
        // Загружаем конфигурацию модели UPS
        ASSERT_TRUE(cfg.load(iniPath, section)) << "Failed to load test INI file: " << iniPath;
        // Инициализируем хранилище
        ASSERT_TRUE(store.init(cfg)) << "Failed to initialize UpsDataStore";
    }
};

// ---------------------------------------------------------------
// init()
// ---------------------------------------------------------------
// Тестируем инициализацию хранилища
TEST_F(UpsDataStoreTest, Init_Success) {
    // SetUp уже всё проверил
    SUCCEED();
}

// ---------------------------------------------------------------
// get()
// ---------------------------------------------------------------
// Получение параметра по OID
TEST_F(UpsDataStoreTest, Get_ReturnsParameter) {
    UpsParameter p;
    ASSERT_TRUE(store.get(cfg.oids().inputVoltageOID, p));
    EXPECT_EQ(p.name, "inputVoltage");
    EXPECT_EQ(p.type, UpsParameterType::Integer);
}
// Получение несуществующего параметра
TEST_F(UpsDataStoreTest, Get_ReturnsFalseForUnknown) {
    UpsParameter p;
    EXPECT_FALSE(store.get("1.2.3.4.5.6.7.8", p));
}

// ---------------------------------------------------------------
// set() — Unknown OID
// ---------------------------------------------------------------
// Попытка установить значение для неизвестного OID
TEST_F(UpsDataStoreTest, Set_ReturnsFalseForUnknownOid) {
    ErrorMessage err;
    EXPECT_FALSE(store.set("1.2.3.4.5.6.7.8.9.10", "123", &err));
    EXPECT_EQ(err, "Unknown OID: 1.2.3.4.5.6.7.8.9.10");
}

// ---------------------------------------------------------------
// set() — Integer параметры
// ---------------------------------------------------------------
// Установка корректного числового значения
TEST_F(UpsDataStoreTest, Set_Integer_AcceptsValidNumber) {
    EXPECT_TRUE(store.set(cfg.oids().inputVoltageOID, "220", &err));
}
// Установка нечислового значения для Integer параметра
TEST_F(UpsDataStoreTest, Set_Integer_RejectsNonNumber) {
    EXPECT_FALSE(store.set(cfg.oids().inputFreqOID, "abc", &err));
}

// ---------------------------------------------------------------
// set() — String параметр
// ---------------------------------------------------------------
// Установка строкового значения для String параметра
TEST_F(UpsDataStoreTest, Set_String_AcceptsAnything) {
    EXPECT_TRUE(store.set(cfg.oids().modelNameOID, "New Model Name", &err));
}

// ---------------------------------------------------------------
// set() — сложные параметры, которые имеют набор допустимых значений
// ---------------------------------------------------------------
// Установка значений для сложного параметра batteryStatus
TEST_F(UpsDataStoreTest, Set_ValueSet_AcceptsNumericValue) {
    const Oid& oid = cfg.oids().batteryStatusOID;
    int anyValue = cfg.definedFields().batteryStatusSet.nameToValue.begin()->second;
    EXPECT_TRUE(store.set(oid, std::to_string(anyValue), &err))
        << "Failed to set numeric value" << err;
}
// Установка именованного значения для сложного параметра batteryStatus
TEST_F(UpsDataStoreTest, Set_ValueSet_AcceptsNameValue) {
    const Oid& oid = cfg.oids().batteryStatusOID;
    std::string anyName = cfg.definedFields().batteryStatusSet.nameToValue.begin()->first;
    EXPECT_TRUE(store.set(oid, anyName, &err)) << "Failed to set named value" << err;
}
// Попытка установить некорректное числовое значение для сложного параметра
TEST_F(UpsDataStoreTest, Set_ValueSet_RejectsInvalidNumber) {
    const Oid& oid = cfg.oids().batteryStatusOID;
    EXPECT_FALSE(store.set(oid, "99999", &err));
}
// Попытка установить некорректное именованное значение для сложного параметра
TEST_F(UpsDataStoreTest, Set_ValueSet_RejectsInvalidName) {
    const Oid& oid = cfg.oids().batteryStatusOID;
    EXPECT_FALSE(store.set(oid, "WrongName", &err));
}
