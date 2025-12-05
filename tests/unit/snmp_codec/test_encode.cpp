#include <gtest/gtest.h>

#include "snmp_ber_writer.h"
#include "snmp_codec.h"
#include "ups_data_store.h"

using namespace snmp;

class SnmpEncoderTest : public ::testing::Test {
protected:
    SnmpGetRequest req;
    std::vector<uint8_t> buf;
    SnmpCodec codec;
    UpsDataStore store;
    ErrorMessage err;

    void SetUp() override { buf.clear(); }

    // Проверка коротких последовательностей
    void expectBytes(std::initializer_list<uint8_t> expected) {
        std::vector<uint8_t> exp(expected);
        EXPECT_EQ(buf, exp);
    }
};
#if 0
// ============================================================
// Часть 1 — ASN.1 INTEGER
// ============================================================

// Тест 1.1: INTEGER = 0
TEST_F(SnmpEncoderTest, EncodeInteger_Zero) {
    req.requestId = 0;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({0x02, 0x01, 0x00});
}

// Тест 1.2: INTEGER > 0
TEST_F(SnmpEncoderTest, EncodeInteger_Positive) {
    req.requestId = 5;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({0x02, 0x01, 0x05});
}

// Тест 1.3: INTEGER < 0
TEST_F(SnmpEncoderTest, EncodeInteger_Negative) {
    req.requestId = -5;  // проверяем отрицательное число
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    // -5 - 0xFB
    expectBytes({0x02, 0x01, 0xFB});
}

// Тест 1.4: INTEGER > 127 должно занимать 2 байта
TEST_F(SnmpEncoderTest, EncodeInteger_BigPositive) {
    req.requestId = 128;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    // 128 кодируется как два байта: 00 80
    expectBytes({0x02, 0x02, 0x00, 0x80});
}

// Тест 1.5: INTEGER большое положительное (4 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_LargePositive4Bytes) {
    req.requestId = 404719978; // 0x18 1F 89 6A
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x18, 0x1F, 0x89, 0x6A  // value
    });
}

// Тест 1.6: INTEGER = -128 (минимальное однобайтовое значение)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative128) {
    // ASN.1: 80
    req.requestId = -128;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x02, 0x01,             // INTEGER, length 1
        0x80                     // -128
    });
}

// Тест 1.7: INTEGER = -129 (требует 2 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative129) {
    // -129 - two's complement: FF 7F
    req.requestId = -129;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x02, 0x02,             // INTEGER, length 2
        0xFF, 0x7F              // -129
    });
}

// Тест 1.8: INTEGER = INT_MAX (2147483647)
TEST_F(SnmpEncoderTest, EncodeInteger_MaxInt32) {
    // 0x7F FF FF FF
    req.requestId = 2147483647;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x7F, 0xFF, 0xFF, 0xFF  // value
    });
}

// Тест 1.9: INTEGER = INT_MIN (-2147483648)
TEST_F(SnmpEncoderTest, EncodeInteger_MinInt32) {
    // two's complement: 0x80 00 00 00
    req.requestId = -2147483648;
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x80, 0x00, 0x00, 0x00  // value
    });
}
#endif

// ============================================================
// Часть 2 — OCTET STRING
// ============================================================

// Тест 2.1: Пустая строка
TEST_F(SnmpEncoderTest, EncodeOctetString_Empty) {
    req.community = "";
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x04, 0x00
    });
}

// Тест 2.2: Текстовая строка - один символ
TEST_F(SnmpEncoderTest, EncodeOctetString_SingleChar) {
    req.community = "A";
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x04, 0x01, 0x41
    });
}

// Тест 2.3:Текстовая строка "Hello"
TEST_F(SnmpEncoderTest, EncodeOctetString_Text) {
    req.community = "Hello";
    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
    expectBytes({
        0x04, 0x05,
        'H','e','l','l','o'
    });
}

// ============================================================
// Часть 3 — ASN.1 NULL
// ============================================================

// Тест 3.1: NULL
// TEST_F(SnmpEncoderTest, EncodeNull) {
//     // TODO
// }

// ============================================================
// Часть 4 — OBJECT IDENTIFIER
// ============================================================

// Тест 4.1: OID базовый
// TEST_F(SnmpEncoderTest, EncodeOid_Simple) {
//     // TODO
// }

// Тест 4.2: OID со значениями > 127
// TEST_F(SnmpEncoderTest, EncodeOid_Long) {
//     // TODO
// }

// ============================================================
// Часть 5 — VarBind (OID + Value)
// ============================================================

// Тест 5.1: VarBind = NULL
// TEST_F(SnmpEncoderTest, EncodeVarBind_NullValue) {
//     // TODO
// }

// Тест 5.2: VarBind = INTEGER
// TEST_F(SnmpEncoderTest, EncodeVarBind_Integer) {
//     // TODO
// }

// Тест 5.3: VarBind = STRING
// TEST_F(SnmpEncoderTest, EncodeVarBind_String) {
//     // TODO
// }

// ============================================================
// Часть 6 — VarBindList (SEQUENCE OF)
// ============================================================

// Тест 6.1: Пустой список
// TEST_F(SnmpEncoderTest, EncodeVarBindList_Empty) {
//     // TODO
// }

// Тест 6.2: Два VarBind
// TEST_F(SnmpEncoderTest, EncodeVarBindList_TwoItems) {
//     // TODO
// }

// ============================================================
// Часть 7 — GetResponse-PDU
// ============================================================

// Тест 7.1: Минимальный PDU
// TEST_F(SnmpEncoderTest, EncodeGetResponsePdu_Basic) {
//     // TODO
// }

// ============================================================
// Часть 8 — Полный SNMP GET-RESPONSE Message
// ============================================================

// Тест 8.1: Полный GET-RESPONSE
// TEST_F(SnmpEncoderTest, EncodeGetResponse_FullMessage) {
//     // TODO
// }
