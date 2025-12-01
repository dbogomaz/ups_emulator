#include <gtest/gtest.h>
#include "snmp/snmp_codec.h"

using namespace snmp;

TEST(SnmpCodecDecodeBasic, DecodeOneOid)
{
    SnmpCodec codec;
    SnmpGetRequest req;

    // SNMPv1 GET request with OID: 1.3.6
    uint8_t data[] = {
        0x30, 0x1e,
        0x02, 0x01, 0x00,
        0x04, 0x06, 'p','u','b','l','i','c',
        0xa0, 0x11,
        0x02, 0x01, 0x01,
        0x02, 0x01, 0x00,
        0x02, 0x01, 0x00,
        0x30, 0x06,
        0x30, 0x04,
        0x06, 0x02, 0x2b, 0x06
    };

    bool ok = codec.decodeGetRequest(data, sizeof(data), req);
    ASSERT_TRUE(ok);

    EXPECT_EQ(req.requestId, 1);
    ASSERT_EQ(req.oids.size(), 1);
    EXPECT_EQ(req.oids[0], "1.3.6");  // decoded OID
}

TEST(SnmpCodecDecodeBasic, DecodeTwoOids)
{
    SnmpCodec codec;
    SnmpGetRequest req;

    uint8_t data[] = {
        0x30, 0x28,
        0x02, 0x01, 0x00,
        0x04, 0x06, 'p','u','b','l','i','c',
        0xa0, 0x1b,
        0x02, 0x01, 0x01,
        0x02, 0x01, 0x00,
        0x02, 0x01, 0x00,
        0x30, 0x0f,

        // VarBind #1
        0x30, 0x07,
        0x06, 0x02, 0x2b, 0x06, // OID 1.3.6
        0x05, 0x00,            // NULL

        // VarBind #2
        0x30, 0x06,
        0x06, 0x03, 0x2b, 0x06, 0x01, // OID 1.3.6.1
        0x05, 0x00
    };

    bool ok = codec.decodeGetRequest(data, sizeof(data), req);
    ASSERT_TRUE(ok);

    EXPECT_EQ(req.requestId, 1);
    
    ASSERT_EQ(req.oids.size(), 2);
    EXPECT_EQ(req.oids[0], "1.3.6");
    EXPECT_EQ(req.oids[1], "1.3.6.1");
}
