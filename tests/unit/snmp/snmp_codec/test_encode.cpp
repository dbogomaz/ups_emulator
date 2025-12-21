#include <gtest/gtest.h>

#include "snmp_ber_writer.h"
#include "snmp_codec.h"
#include "ups_data_store.h"

class SnmpEncoderTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    SnmpCodec codec;
    BerWriter* w = nullptr;

    UpsModelConfig cfg;
    std::string dataDir = std::string(TEST_DATA_DIR) + "/snmp_codec";
    // Полный путь к файлу тестовых данных
    std::string data(const std::string& name) const { return dataDir + "/" + name; }

    void SetUp() override {
        buf.clear();
        w = new BerWriter(buf);
    }

    void TearDown() override { delete w; }

    // Проверка коротких последовательностей
    void expectBytes(std::initializer_list<uint8_t> expected) {
        std::vector<uint8_t> exp(expected);
        EXPECT_EQ(buf, exp);
    }
};

#if 1  // Часть 1 — ASN.1 INTEGER
// ============================================================
// Часть 1 — ASN.1 INTEGER
// ============================================================

// Тест 1.1: INTEGER = 0
TEST_F(SnmpEncoderTest, EncodeInteger_Zero) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 0);
    expectBytes({ 0x02, 0x01, 0x00 });
}

// Тест 1.2: INTEGER > 0
TEST_F(SnmpEncoderTest, EncodeInteger_Positive) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 5);
    expectBytes({ 0x02, 0x01, 0x05 });
}

// Тест 1.3: INTEGER < 0
TEST_F(SnmpEncoderTest, EncodeInteger_Negative) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -5);
    expectBytes({ 0x02, 0x01, 0xFB });
}

// Тест 1.4: INTEGER > 127 должно занимать 2 байта
TEST_F(SnmpEncoderTest, EncodeInteger_BigPositive) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 128);
    // 128 = 0x00 0x80 (чтобы не было знакового бита)
    // clang-format off
    expectBytes({
        0x02, 0x02,  // TAG + length
        0x00, 0x80
    });
    // clang-format on
}

// Тест 1.5: INTEGER большое положительное (4 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_LargePositive4Bytes) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 404719978);  // 0x18 1F 89 6A
    // clang-format off
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x18, 0x1F, 0x89, 0x6A  // value
    });
    // clang-format on
}

// Тест 1.6: INTEGER = -128 (минимальное однобайтовое значение)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative128) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -128);
    // clang-format off
    expectBytes({
        0x02, 0x01,  // INTEGER, length 1
        0x80         // -128
    });
    // clang-format on
}

// Тест 1.7: INTEGER = -129 (требует 2 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative129) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -129);
    // clang-format off
    expectBytes({
        0x02, 0x02,  // INTEGER, length 2
        0xFF, 0x7F   // -129
    });
    // clang-format on
}

// Тест 1.8: INTEGER = INT_MAX (2147483647)
TEST_F(SnmpEncoderTest, EncodeInteger_MaxInt32) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 2147483647);
    // clang-format off
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x7F, 0xFF, 0xFF, 0xFF  // value
    });
    // clang-format on
}

// Тест 1.9: INTEGER = INT_MIN (-2147483648)
TEST_F(SnmpEncoderTest, EncodeInteger_MinInt32) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -2147483648LL);
    // clang-format off
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x80, 0x00, 0x00, 0x00  // value
    });
    // clang-format on
}

#endif

#if 1  // Часть 2 — OCTET STRING
// ============================================================
// Часть 2 — OCTET STRING
// ============================================================

// Тест 2.1: Пустая строка
TEST_F(SnmpEncoderTest, EncodeOctetString_Empty) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "");
    expectBytes({ 0x04, 0x00 });
}

// Тест 2.2: Текстовая строка - один символ
TEST_F(SnmpEncoderTest, EncodeOctetString_SingleChar) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "A");
    expectBytes({ 0x04, 0x01, 0x41 });
}

// Тест 2.3:Текстовая строка "Hello"
TEST_F(SnmpEncoderTest, EncodeOctetString_Text) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "Hello");
    expectBytes({ 0x04, 0x05, 'H', 'e', 'l', 'l', 'o' });
}

#endif

#if 1  // Часть 3 — ASN.1 NULL
// ============================================================
// Часть 3 — ASN.1 NULL
// ============================================================
TEST_F(SnmpEncoderTest, EncodeNull) {
    SnmpCodecTestAccess::encodeNull(codec, *w);
    expectBytes({
        0x05, 0x00  // TAG_NULL, length=0
    });
}
#endif

#if 1  // Часть 4 — OBJECT IDENTIFIER (OID)
// ============================================================
// Часть 4 — OBJECT IDENTIFIER (OID)
// ============================================================

// Тест 4.1: OID базовый
TEST_F(SnmpEncoderTest, EncodeOid_Simple) {
    SnmpCodecTestAccess::encodeOid(codec, *w, "1.3.6.1.4.1.9999.1");
    // clang-format off
    expectBytes({
        0x06, 0x08,              // TAG_OID, length=8
        0x2B,                    // 1*40 + 3
        0x06, 0x01, 0x04, 0x01,  // path
        0xCE, 0x0F,              // 9999 - CE 0F
        0x01                     // last component
    });
    // clang-format on
}

// Тест 4.2: OID со значениями > 127
TEST_F(SnmpEncoderTest, EncodeOid_Long) {
    SnmpCodecTestAccess::encodeOid(codec, *w, "1.3.6.1.4.1.5000000.1");
    // clang-format off
    expectBytes({
        0x06, 0x0A,              // TAG_OID, length=10
        0x2B,                    // 1*40 + 3
        0x06, 0x01, 0x04, 0x01,  // 6.1.4.1
        0x82, 0xB1, 0x96, 0x40,  // 5,000,000 encoded base-128
        0x01                     // 1
    });
    // clang-format on
}

// Тест 4.3: Все компоненты OID < 128 (простейший случай)
TEST_F(SnmpEncoderTest, EncodeOid_AllSmall) {
    SnmpCodecTestAccess::encodeOid(codec, *w, "1.3.6.1.2.3.4.5.6");
    // clang-format off
    expectBytes({
        0x06, 0x08,  // TAG + length
        0x2B,        // first=1*40+3
        0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
    });
    // clang-format on
}

// Тест 4.4 Пустой OID
TEST_F(SnmpEncoderTest, EncodeOid_EmptyString) {
    SnmpCodecTestAccess::encodeOid(codec, *w, "");
    expectBytes({ 0x06, 0x00 });
}

// Тест 4.5 В OID только один компонент "1"
TEST_F(SnmpEncoderTest, EncodeOid_SingleComponent) {
    SnmpCodecTestAccess::encodeOid(codec, *w, "1");
    expectBytes({ 0x06, 0x00 });
}

#endif

#if 1  // Часть 5 — VarBind (OID + Value)
// ============================================================
// Часть 5 — VarBind (OID + Value)
// ============================================================

// Тест 5.1: VarBind = NULL (param == nullptr)
TEST_F(SnmpEncoderTest, EncodeVarBind_NullValue) {
    // OID без параметра - param == nullptr
    SnmpCodecTestAccess::encodeVarBind(codec, *w, "1.3.6.1.4.1.9999.1", nullptr);
    // clang-format off
    expectBytes({
        0x30, 0x0C,                                     // SEQUENCE, length = 12
        0x06, 0x08,                                     // OID, length = 8
        0x2B, 0x06, 0x01, 0x04, 0x01, 0xCE, 0x0F, 0x01, // OID
        0x05, 0x00 // NULL
    });
    // clang-format on
}

// Тест 5.2: VarBind = INTEGER
TEST_F(SnmpEncoderTest, EncodeVarBind_Integer) {
    UpsParameter p;
    p.type = UpsParameterType::Integer;
    p.value = "123";
    SnmpCodecTestAccess::encodeVarBind(codec, *w, "1.3.6.1.4.1.9999.2", &p);
    // clang-format off
    expectBytes({
        0x30, 0x0D,                                     // SEQUENCE, len = 13
        0x06, 0x08,                                     // OID, len 8
        0x2B, 0x06, 0x01, 0x04, 0x01, 0xCE, 0x0F, 0x02, // OID
        0x02, 0x01, 0x7B                                // INTEGER 123
    });
    // clang-format on
}

// Тест 5.3: VarBind = STRING
TEST_F(SnmpEncoderTest, EncodeVarBind_String) {
    UpsParameter p;
    p.type = UpsParameterType::String;
    p.value = "UPS";
    SnmpCodecTestAccess::encodeVarBind(codec, *w, "1.3.6.1.4.1.9999.3", &p);
    // clang-format off
    expectBytes({
        0x30, 0x0F, // SEQUENCE, len = 15
        0x06, 0x08, // OID
        0x2B, 0x06, 0x01, 0x04, 0x01, 0xCE, 0x0F, 0x03, 0x04, 0x03, 'U', 'P', 'S' // OCTET STRING "UPS"
    });
    // clang-format on
}

//  Тест 5.4: VarBind = неизвестный тип
TEST_F(SnmpEncoderTest, EncodeVarBind_DefaultCase) {
    UpsParameter p;
    p.type = static_cast<UpsParameterType>(999);
    p.value = "ignored";
    SnmpCodecTestAccess::encodeVarBind(codec, *w, "1.3.6.1.4.1.9999.10", &p);
    // Ожидаем VarBind: OID + NULL
    // clang-format off
    expectBytes({
        0x30, 0x0C,
        0x06, 0x08,
        0x2B, 0x06, 0x01, 0x04, 0x01,
        0xCE, 0x0F, 0x0A, // 10
        0x05, 0x00
    });
    // clang-format on
}

#endif

#if 1  // Часть 6 — VarBindList (SEQUENCE OF)
// ============================================================
// Часть 6 — VarBindList (SEQUENCE OF)
// ============================================================

// Тест 6.1: Пустой список
TEST_F(SnmpEncoderTest, EncodeVarBindList_Empty) {
    // Пустой список OID
    std::vector<Oid> oids;
    // Загружаем конфиг
    ASSERT_TRUE(cfg.load(data("varbindlist_basic.ini"), "APC"));
    UpsDataStore store;
    ASSERT_TRUE(store.init(cfg));
    // Вызываем encodeVarBindList через тестовый доступ
    SnmpCodecTestAccess::encodeVarBindList(codec, *w, oids, store);
    // Ожидаем SEQUENCE длиной 0
    expectBytes({ 0x30, 0x00 });
}

// Тест 6.2: Два VarBind
TEST_F(SnmpEncoderTest, EncodeVarBindList_TwoItems) {
    // 1) Загружаем конфиг
    ASSERT_TRUE(cfg.load(data("varbindlist_basic.ini"), "APC"));
    UpsDataStore store;
    ASSERT_TRUE(store.init(cfg));

    // 2) Устанавливаем два значения
    store.set(cfg.oids().inputVoltageOID, "230");  // inputVoltage 1.3.6.1.4.1.318.1.1.1.3.2.1.0
    store.set(cfg.oids().batteryStatusOID, "2");   // batteryStatus 1.3.6.1.4.1.318.1.1.1.2.1.1.0

    // 3) Список OID для VarBindList
    std::vector<Oid> oids = { cfg.oids().inputVoltageOID, cfg.oids().batteryStatusOID };

    // 4) Кодируем
    SnmpCodecTestAccess::encodeVarBindList(codec, *w, oids, store);
    // clang-format off
    expectBytes({
        // --- VarBindList (SEQUENCE) ---
        0x30, 0x2B,  // len 43
            // --- VarBind #1 ---
            0x30, 0x14, // len 20
                0x06, 0x0E, // len 14
                    0x2B,                                         // 1*40+3
                        0x06, 0x01, 0x04, 0x01,                   // 6.1.4.1
                        0x82, 0x3E,                               // 318
                        0x01, 0x01, 0x01, 0x03, 0x02, 0x01, 0x00, // 1.1.1.3.2.1.0
                0x02, 0x02, 0x00, 0xE6, // INTEGER 230
            // --- VarBind #2 ---
            0x30, 0x13, // len 19
                0x06, 0x0E, // len 14
                    0x2B,                                         // 1*40+3
                        0x06, 0x01, 0x04, 0x01,                   // 6.1.4.1
                        0x82, 0x3E,                               // 318
                        0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x00, // 1.1.1.2.1.1.0
                0x02, 0x01, 0x02 // INTEGER 2
    });
    // clang-format on
}
#endif

#if 1  // Часть 7 — GetResponse-PDU
// ============================================================
// Часть 7 — GetResponse-PDU
// ============================================================

// Тест 7.1: Минимальный PDU
TEST_F(SnmpEncoderTest, EncodeGetResponsePdu_Basic) {
    SnmpGetRequest req;
    req.requestId = 123;
    UpsDataStore store;    // пустой store
    std::vector<Oid> oids; // пустой список
    SnmpCodecTestAccess::encodeGetResponsePdu(codec, *w, req, store);
    // clang-format off
    expectBytes({
        0xA2, 0x0B,       // PDU len = 11
        0x02, 0x01, 0x7B, // request-id = 123
        0x02, 0x01, 0x00, // error-status
        0x02, 0x01, 0x00, // error-index
        0x30, 0x00        // empty varbind list
    });
    // clang-format on
}
#endif

#if 1  // Часть 8 — Полный SNMP GET-RESPONSE Message
// ============================================================
// Часть 8 — Полный SNMP GET-RESPONSE Message
// ============================================================

// Тест 8.1: Полный GET-RESPONSE
TEST_F(SnmpEncoderTest, EncodeGetResponse_FullMessage) {
    ASSERT_TRUE(cfg.load(data("varbindlist_basic.ini"), "APC"));
    UpsDataStore store;
    ASSERT_TRUE(store.init(cfg));
    store.set(cfg.oids().batteryStatusOID, "2");
    SnmpGetRequest req;
    req.version = SnmpVersion::V_1;
    req.community = "public";
    req.requestId = 123;
    req.oids = { cfg.oids().batteryStatusOID };
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf));
    // clang-format off
    expectBytes({
        0x30, 0x2D,             // full message, len=45
        0x02, 0x01, 0x00,       // version=0
        0x04, 0x06, 'p','u','b','l','i','c',
        0xA2, 0x20,             // GetResponse-PDU, len=32
            0x02, 0x01, 0x7B,   // request-id=123
            0x02, 0x01, 0x00,   // error-status
            0x02, 0x01, 0x00,   // error-index
            0x30, 0x15,         // VarBindList, len=21
                0x30, 0x13,     // VarBind, len=19
                    0x06, 0x0E, // OID len=14
                        0x2B,
                        0x06, 0x01, 0x04, 0x01,
                        0x82, 0x3E,
                        0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x00,
                    0x02, 0x01, 0x02 // INTEGER 2
    });
    // clang-format on
}
#endif
