#include <gtest/gtest.h>
#include "snmp/snmp_codec.h"

using namespace snmp;

TEST(SnmpCodecDecodeBasic, DecodeOneOid) {
    SnmpCodec codec;
    SnmpGetRequest req;
    ErrorMessage err;

    // SNMPv1 GET request with OID: 1.3.6
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

    bool ok = codec.decodeGetRequest(data, sizeof(data), req, &err);
    ASSERT_TRUE(ok) << "Decode failed: " << err;

    ASSERT_EQ(req.community, "public") << "Wrong community string " << req.community;
    EXPECT_EQ(req.requestId, 0x181F896A) << "Wrong requestId " << req.requestId;
    ASSERT_EQ(req.oids.size(), 1) << "Wrong number of OIDs " << req.oids.size();
    EXPECT_EQ(req.oids[0], "1.3.6.1.2") << "Wrong OID " << req.oids[0];
}

#if 0
TEST(SnmpCodecDecodeBasic, DecodeTwoOids)
{
    SnmpCodec codec;
    SnmpGetRequest req;
    ErrorMessage err;

    // SNMPv1 GET request with OIDs: 1.3.6 and 1.3.6.1
    uint8_t data[] = {
        0x30, 0x39,
        0x02, 0x01, 0x00,
        0x04, 0x06, 'p','u','b','l','i','c',
        0xA0, 0x2C,
        0x02, 0x04, 0x16, 0x0B, 0x76, 0x9C,  // request-id large
        0x02, 0x01, 0x00,                    // error-status
        0x02, 0x01, 0x00,                    // error-index

        // VarBindList
        0x30, 0x1E,

        // VarBind #1
        0x30, 0x0C,
        0x06, 0x08, 0x2B, 0x06, 0x01, 0x02, 0x01, 0x36, 0x01,
        0x05, 0x00,

        // VarBind #2
        0x30, 0x0E,
        0x06, 0x0A, 0x2B, 0x06, 0x01, 0x02, 0x01, 0x36, 0x04, 0x00,
        0x05, 0x00
    };


    bool ok = codec.decodeGetRequest(data, sizeof(data), req, &err);
    ASSERT_TRUE(ok) << "Decode failed: " << err;

    EXPECT_EQ(req.requestId, 369707772) << "Wrong requestId: " << req.requestId;
    
    ASSERT_EQ(req.oids.size(), 2) << "Wrong number of OIDs: " << req.oids.size();
    EXPECT_EQ(req.oids[0], "1.3.6.1.2.1.54.1") << "Wrong OID: " << req.oids[0];
    EXPECT_EQ(req.oids[1], "1.3.6.1.2.1.54.4.0") << "Wrong OID: " << req.oids[1];
}
#endif