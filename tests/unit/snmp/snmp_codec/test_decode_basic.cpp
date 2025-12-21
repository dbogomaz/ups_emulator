#include <gtest/gtest.h>

#include "snmp_codec.h"

class SnmpCodecTest : public ::testing::Test {
protected:
    SnmpCodec codec;
    SnmpGetRequest req;
    ErrorMessage err;
};

// ------------------------------------------------------------
// Тестируем декодирование SNMP GET-запросов (SNMPv1) с одним и двумя OID'ами
// ------------------------------------------------------------
// Пример запроса с одним OID:
TEST_F(SnmpCodecTest, DecodeOneOid) {
    // SNMPv1 GET request with OID: 1.3.6.1.2
    // clang-format off
    uint8_t data[] = {
        0x30, 0x25, // SEQUENCE length 37
            0x02, 0x01, 0x00, // version
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA0, 0x18, // GetRequest PDU length 24
                0x02, 0x04, 0x18, 0x1f, 0x89, 0x6a, // request-id 404719978 length 4  
                0x02, 0x01, 0x00, // error-status
                0x02, 0x01, 0x00, // error-index
                0x30, 0x0a, // VarBindList length 10
                    0x30, 0x08, // VarBind #1 length 8
                        0x06, 0x04, 0x2b, 0x06, 0x01, 0x02, // OID 1.3.6.1.2 length 4
                        0x05, 0x00 // Value = NULL
    };
    // clang-format on
    bool ok = codec.decodeGetRequest(data, sizeof(data), req, &err);
    ASSERT_TRUE(ok) << "Decode failed: " << err;
    ASSERT_EQ(req.community, "public") << "Wrong community string: " << req.community;
    EXPECT_EQ(req.requestId, 0x181F896A) << "Wrong requestId: " << req.requestId;
    ASSERT_EQ(req.oids.size(), 1) << "Wrong number of OIDs: " << req.oids.size();
    EXPECT_EQ(req.oids[0], "1.3.6.1.2") << "Wrong OID: " << req.oids[0];
}

// Пример запроса с двумя OID'ами:
TEST_F(SnmpCodecTest, DecodeTwoOids) {
    // SNMPv1 GET request with OIDs: 1.3.6 and 1.3.6.1
    // clang-format off
    uint8_t data[] = {
        0x30, 0x2f, // SEQUENCE length 47
            0x02, 0x01, 0x00, // version
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA0, 0x22, // GetRequest PDU length 34
                0x02, 0x04, 0x32, 0x10, 0xec, 0x52, // request-id 839969874 length 4  
                0x02, 0x01, 0x00, // error-status
                0x02, 0x01, 0x00, // error-index
                0x30, 0x14, // VarBindList length 20
                    0x30, 0x08, // VarBind #1 length 8
                        0x06, 0x04, 0x2b, 0x06, 0x01, 0x02, // OID 1.3.6.1.2 length 4
                        0x05, 0x00, // Value = NULL
                    0x30, 0x08, // VarBind #2 length 8
                        0x06, 0x04, 0x2b, 0x06, 0x01, 0x04, // OID 1.3.6.1.4 length 4
                        0x05, 0x00 // Value = NULL
 
    };
    // clang-format on
    bool ok = codec.decodeGetRequest(data, sizeof(data), req, &err);
    ASSERT_TRUE(ok) << "Decode failed: " << err;
    ASSERT_EQ(req.community, "public") << "Wrong community string: " << req.community;
    EXPECT_EQ(req.requestId, 0x3210ec52) << "Wrong requestId: " << req.requestId;
    ASSERT_EQ(req.oids.size(), 2) << "Wrong number of OIDs: " << req.oids.size();
    EXPECT_EQ(req.oids[0], "1.3.6.1.2") << "Wrong OID: " << req.oids[0];
    EXPECT_EQ(req.oids[1], "1.3.6.1.4") << "Wrong OID: " << req.oids[1];
}
