#include <gtest/gtest.h>
#include "snmp/snmp_codec.h"

using namespace snmp;

class SnmpCodecDecodeErrors : public ::testing::Test {
protected:
    SnmpCodec codec;
    SnmpGetRequest req;
    ErrorMessage err;

    // Helpers для удобного вызова декодирования
    bool decode(const uint8_t* data, size_t size) {
        req = SnmpGetRequest{}; // сброс
        err.clear();
        return codec.decodeGetRequest(data, size, req, &err);
    }
};

// ------------------------------------------------------------
// Тестируем декодирование SNMP GET-запросов (SNMPv1) с ошибками
// ------------------------------------------------------------
// Пример запроса с неверным верхнеуровневым тегом:
TEST_F(SnmpCodecDecodeErrors, InvalidTopLevelTag) {
    // Первый байт НЕ 0x30 (SEQUENCE) → ошибка
    uint8_t data[] = {
        0x31, 0x05,      // WRONG tag
        0x02, 0x01, 0x00 // version = 0 (хотя мы даже сюда не должны дойти)
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// Пример запроса с длиной SEQUENCE, превышающей размер буфера:
TEST_F(SnmpCodecDecodeErrors, SequenceLengthExceedsBuffer) {
    // Корректный тег 0x30, НО длина 50 — больше, чем реально есть в массиве
    uint8_t data[] = {
        0x30, 0x32,      // length = 50 (0x32), но данных всего 3 байта
        0x02, 0x01, 0x00 // version = 0
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "SEQUENCE length exceeds buffer");
}

// Пример запроса с неверным тегом версии (должен быть INTEGER = 0x02)
TEST_F(SnmpCodecDecodeErrors, InvalidVersionTag) {
    uint8_t data[] = {
        0x30, 0x07,          // SEQUENCE, length=7
            0x05,            // WRONG tag (should be 0x02)
            0x01, 0x00,      // version data (ignored)
            0x04, 0x01, 'x', // community string
            0xA0, 0x00       // empty GetRequest
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Некорректная длина версии: length = 0
TEST_F(SnmpCodecDecodeErrors, InvalidVersionLengthZero) {
    uint8_t data[] = {
        0x30, 0x02,     // SEQUENCE
            0x02, 0x00, // INTEGER tag, length=0 (ошибка)
            // нет данных
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "INTEGER content exceeds buffer");
}

// Пример запроса с неверной длиной строки сообщества (длина превышает буфер)
TEST_F(SnmpCodecDecodeErrors, InvalidCommunityLength) {
    uint8_t data[] = {
        0x30, 0x06,                 // SEQUENCE len=6
            0x02, 0x01, 0x00,       // version = 0
            0x04, 0x05, 'p','u','b' // length = 5, but only 3 bytes
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "OCTET STRING length exceeds buffer");
}

// Неверный тег GetRequest (должен быть A0)
TEST_F(SnmpCodecDecodeErrors, InvalidGetRequestTag) {
    uint8_t data[] = {
        0x30, 0x08,           // SEQUENCE len=8
            0x02, 0x01, 0x00, // version = 0
            0x04, 0x01, 'x',  // community = "x"
            0xA1, 0x00        // WRONG tag (should be A0)
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0xA0, got 0xA1");
}


// PDU length указывает за пределы верхнего SEQUENCE → ошибка
TEST_F(SnmpCodecDecodeErrors, PduLengthExceedsBounds) {
    uint8_t data[] = {
        0x30, 0x08,           // SEQUENCE, length = 7 bytes
            0x02, 0x01, 0x00, // version = 0
            0x04, 0x01, 'x',  // community = "x"
            0xA0, 0x05        // PDU length = 5 (но реально доступно всего 1 байт)
            // нет 5 байт информации — это выход за msgEnd
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "PDU length exceeds message bounds");
}

// Пример запроса с неверным тегом request-id (должен быть INTEGER = 0x02)
TEST_F(SnmpCodecDecodeErrors, InvalidRequestIdTag) {
    uint8_t data[] = {
        0x30, 0x0B,              // SEQUENCE, length = 11
            0x02, 0x01, 0x00,    // version
            0x04, 0x01, 'x',     // community
            0xA0, 0x03,          // PDU length = 3
                0x05, 0x01, 0x00 // WRONG TAG (should be 0x02)
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Пример запроса с неверным тегом error-status (должен быть INTEGER = 0x02)
TEST_F(SnmpCodecDecodeErrors, InvalidErrorStatusTag) {
    uint8_t data[] = {
        0x30, 0x0D,               // SEQUENCE length = 13
            0x02, 0x01, 0x00,     // version = 0
            0x04, 0x01, 'x',      // community = "x"
            0xA0, 0x04,           // PDU length = 4
                0x02, 0x01, 0x01, // request-id INTEGER (value 1)
                0x05, 0x00        // WRONG TAG (should be INTEGER 0x02)
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Пример запроса с неверным тегом error-index (должен быть INTEGER = 0x02)
TEST_F(SnmpCodecDecodeErrors, InvalidErrorIndexTag) {
    uint8_t data[] = {
        0x30, 0x10,               // SEQUENCE length = 16
            0x02, 0x01, 0x00,     // version
            0x04, 0x01, 'x',      // community
            0xA0, 0x08,           // PDU length = 8
                0x02, 0x01, 0x01, // request-id
                0x02, 0x01, 0x00, // error-status
                0x05, 0x00        // WRONG TAG instead of INTEGER
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Неверный тег VarBind (должен быть SEQUENCE = 0x30)
TEST_F(SnmpCodecDecodeErrors, InvalidVarBindTag) {
    uint8_t data[] = {
        0x30, 0x17,                // SEQUENCE length = 23
            0x02, 0x01, 0x00,      // version = 0
            0x04, 0x01, 'x',       // community = "x"
            0xA0, 0x0F,            // PDU length = 15
                0x02, 0x01, 0x01,  // request-id = 1
                0x02, 0x01, 0x00,  // error-status = 0
                0x02, 0x01, 0x00,  // error-index = 0
                0x30, 0x04,        // VarBindList length = 4
                    0x31, 0x02,    // WRONG VarBind tag (should be 0x30)
                        0x06, 0x00 // dummy
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// Неверный тег VarBindList (должен быть SEQUENCE = 0x30)
TEST_F(SnmpCodecDecodeErrors, InvalidVarBindListTag) {
    uint8_t data[] = {
        0x30, 0x17,                // SEQUENCE length = 23
            0x02, 0x01, 0x00,      // version = 0
            0x04, 0x01, 'x',       // community = "x"
            0xA0, 0x0F,            // PDU length = 15
                0x02, 0x01, 0x01,  // request-id = 1
                0x02, 0x01, 0x00,  // error-status = 0
                0x02, 0x01, 0x00,  // error-index = 0
                0x31, 0x04,        // WRONG VarBindList tag (should be 0x30)
                    0x30, 0x02,    // VarBind #1
                        0x06, 0x00 // dummy
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// PDU length указывает за предел msgEnd (ошибка "PDU length exceeds message bounds")
TEST_F(SnmpCodecDecodeErrors, PduLengthTooBig) {
    uint8_t data[] = {
        0x30, 0x0A,              // SEQUENCE length = 10
            0x02, 0x01, 0x00,    // version = 0
            0x04, 0x01, 'x',     // community = "x"
            0xA0, 0x05,          // PDU length = 5, но должно быть 3
                0x00, 0x00  // request-id = 1
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    ASSERT_EQ(err, "PDU length exceeds message bounds");
}
