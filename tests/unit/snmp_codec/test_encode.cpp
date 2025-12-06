#include <gtest/gtest.h>

#include "snmp_ber_writer.h"
#include "snmp_codec.h"
#include "ups_data_store.h"

using namespace snmp;

class SnmpEncoderTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    SnmpCodec codec;
    BerWriter* w = nullptr;

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
#if 0  // Часть 1 — ASN.1 INTEGER
// ============================================================
// Часть 1 — ASN.1 INTEGER
// ============================================================

// Тест 1.1: INTEGER = 0
TEST_F(SnmpEncoderTest, EncodeInteger_Zero) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 0);
    expectBytes({0x02, 0x01, 0x00});
}

// Тест 1.2: INTEGER > 0
TEST_F(SnmpEncoderTest, EncodeInteger_Positive) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 5);
    expectBytes({0x02, 0x01, 0x05});
}

// Тест 1.3: INTEGER < 0
TEST_F(SnmpEncoderTest, EncodeInteger_Negative) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -5);
    expectBytes({0x02, 0x01, 0xFB});
}

// Тест 1.4: INTEGER > 127 должно занимать 2 байта
TEST_F(SnmpEncoderTest, EncodeInteger_BigPositive) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 128);
    // 128 = 0x00 0x80 (чтобы не было знакового бита)
    expectBytes({0x02, 0x02,  // TAG + length
                 0x00, 0x80});
}

// Тест 1.5: INTEGER большое положительное (4 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_LargePositive4Bytes) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 404719978);  // 0x18 1F 89 6A
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x18, 0x1F, 0x89, 0x6A  // value
    });
}

// Тест 1.6: INTEGER = -128 (минимальное однобайтовое значение)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative128) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -128);
    expectBytes({
        0x02, 0x01,  // INTEGER, length 1
        0x80         // -128
    });
}

// Тест 1.7: INTEGER = -129 (требует 2 байта)
TEST_F(SnmpEncoderTest, EncodeInteger_Negative129) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -129);
    expectBytes({
        0x02, 0x02,  // INTEGER, length 2
        0xFF, 0x7F   // -129
    });
}

// Тест 1.8: INTEGER = INT_MAX (2147483647)
TEST_F(SnmpEncoderTest, EncodeInteger_MaxInt32) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, 2147483647);
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x7F, 0xFF, 0xFF, 0xFF  // value
    });
}

// Тест 1.9: INTEGER = INT_MIN (-2147483648)
TEST_F(SnmpEncoderTest, EncodeInteger_MinInt32) {
    SnmpCodecTestAccess::encodeInteger(codec, *w, -2147483648LL);
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x80, 0x00, 0x00, 0x00  // value
    });
}

#endif

#if 1  // Часть 2 — OCTET STRING
// ============================================================
// Часть 2 — OCTET STRING
// ============================================================

// Тест 2.1: Пустая строка
TEST_F(SnmpEncoderTest, EncodeOctetString_Empty) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "");
    expectBytes({0x04, 0x00});
}

// Тест 2.2: Текстовая строка - один символ
TEST_F(SnmpEncoderTest, EncodeOctetString_SingleChar) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "A");
    expectBytes({0x04, 0x01, 0x41});
}

// Тест 2.3:Текстовая строка "Hello"
TEST_F(SnmpEncoderTest, EncodeOctetString_Text) {
    SnmpCodecTestAccess::encodeOctetString(codec, *w, "Hello");
    expectBytes({0x04, 0x05, 'H', 'e', 'l', 'l', 'o'});
}

#endif

// ============================================================
// Часть 3 — ASN.1 NULL
// ============================================================

// TEST_F(SnmpEncoderTest, EncodeNull) {
//     req.requestId = 0;
//     // временно encodeGetResponse вызывает encodeNull()
//     ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//     expectBytes({
//         0x05, 0x00  // ASN.1 NULL
//     });
// }

// ============================================================
// Часть 4 — OBJECT IDENTIFIER
// ============================================================

// TEST_F(SnmpEncoderTest, EncodeOid_Simple) {
//     req.oids = { "1.3.6.1.4.1.9999.1" };
//     ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//     expectBytes({
//         0x06, 0x08,
//         0x2B,       // 1*40 + 3
//         0x06, 0x01, 0x04, 0x01,
//         0xCE, 0x0F, // 9999 encoded in base-128
//         0x01        // last component
//     });
// }

// Тест 4.2: OID со значениями > 127
// TEST_F(SnmpEncoderTest, EncodeOid_Long) {
//    req.oids = { "1.3.6.1.4.1.5000000.1" };
//    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//    expectBytes({
//        0x06,       // TAG_OID
//        0x0A,       // length = 10
//        0x2B,       // 1*40 + 3
//        0x06, 0x01, 0x04, 0x01,
//        // base-128 encoding of 5,000,000
//        0x82, 0xB1, 0x96, 0x40,
//        0x01        // last component
//    });
//}

//// Тест 4.3: Все компоненты OID < 128 (простейший случай)
// TEST_F(SnmpEncoderTest, EncodeOid_AllSmall) {
//     req.oids = { "1.3.6.1.2.3.4.5.6" };
//     ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//     expectBytes({
//         0x06,       // TAG_OID
//         0x08,       // length = 8
//         0x2B,       // 1*40 + 3
//         0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
//     });
// }

// ============================================================
// Часть 5 — VarBind (OID + Value)
// ============================================================

// Тест 5.1: VarBind = NULL (param == nullptr)
// TEST_F(SnmpEncoderTest, EncodeVarBind_NullValue) {
//    req.oids = { "1.3.6.1.4.1.9999.1" };
//    // специально создаём store без параметра - param == nullptr
//    ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//    expectBytes({
//        0x30, 0x0C,                // SEQUENCE (VarBind), length 12
//        0x06, 0x08,                // OID, len=8
//        0x2B, 0x06, 0x01, 0x04, 0x01, 0xCE, 0x0F, 0x01,
//        0x05, 0x00                 // NULL
//    });
//}

//// Тест 5.2: VarBind = INTEGER
// TEST_F(SnmpEncoderTest, EncodeVarBind_Integer) {
//     req.oids = { "1.3.6.1.4.1.9999.2" };
//     UpsParameter p;
//     p.type = UpsParameterType::Integer;
//     p.value = "123";
//     store.set("1.3.6.1.4.1.9999.2", p.value);
//     ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//     expectBytes({
//         0x30, 0x0D,
//         0x06, 0x08,
//         0x2B, 0x06, 0x01, 0x04, 0x01, 0xCE, 0x0F, 0x02,
//         0x02, 0x01, 0x7B   // INTEGER 123 (0x7B)
//     });
// }

// // Тест 5.3: VarBind = STRING
// TEST_F(SnmpEncoderTest, EncodeVarBind_String) {
//     req.oids = { "1.3.6.1.4.1.9999.3" };
//     UpsParameter p;
//     p.type = UpsParameterType::String;
//     p.value = "UPS";
//     store.set("1.3.6.1.4.1.9999.3", p.value);
//     ASSERT_TRUE(codec.encodeGetResponse(req, store, buf, &err));
//     expectBytes({
//         0x30, 0x0E,                 // seq len 14
//         0x06, 0x08,
//         0x2B, 0x06, 0x01, 0x04, 0x01, 0x92, 0x0F, 0x03,
//         0x04, 0x03, 'U', 'P', 'S'   // OCTET STRING "UPS"
//     });
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
