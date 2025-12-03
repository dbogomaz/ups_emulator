#ifndef SNMP_CODEC_H
#define SNMP_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

#include "ups_data_store.h"
#include "ups_types.h"

namespace snmp {

// ------------------------------------------------------------
// Parsed SNMP GET Request (SNMPv1)
// ------------------------------------------------------------
struct SnmpGetRequest {
    int requestId{0};               // request-id
    std::string community{};        // community string ("public")
    std::vector<std::string> oids;  // list of OIDs from varbind-list
};

// ------------------------------------------------------------
// SNMP Codec (ASN.1 BER encoder/decoder for SNMP v1)
// ------------------------------------------------------------
class SnmpCodec {
public:
    // Decode SNMP GET Request (SNMP v1)
    bool decodeGetRequest(const uint8_t* data, size_t size,
                          SnmpGetRequest& outReq,
                          std::string* err = 0);
#if 0
    // Encode SNMP GET Response (SNMP v1)
    std::vector<uint8_t> encodeGetResponse(int requestId,
                                           const std::vector<std::string>& oids,
                                           const UpsDataStore& store);
#endif

private:
    // ============================================================
    // ASN.1 TAG DEFINITIONS
    // ============================================================
    enum : uint8_t {
        TAG_SEQUENCE      = 0x30,
        TAG_INTEGER       = 0x02,
        TAG_OCTETSTRING   = 0x04,
        TAG_NULL          = 0x05,
        TAG_OID           = 0x06,
        TAG_GETREQUEST    = 0xA0,
        TAG_GETRESPONSE   = 0xA2
    };

    // ============================================================
    // Low-level ASN.1 decoding helpers
    // ============================================================
    bool readTagAndLength(const uint8_t*& p, const uint8_t* end, 
                          uint8_t expectedTag, size_t& outLen, std::string& err);

    bool readSequence(const uint8_t*& p, const uint8_t* end,
                      const uint8_t*& seqEnd, std::string& err);      
    bool readInteger(const uint8_t*& p, const uint8_t* end,
                     int& outValue, std::string& err);
    bool readOctetString(const uint8_t*& p, const uint8_t* end,
                         std::string& outStr, std::string& err);
    bool readPdu(const uint8_t*& p, const uint8_t* end, 
                 const uint8_t*& pduEnd, std::string& err);
    bool readOid(const uint8_t*& p, const uint8_t* end,
                 std::string& outOid, std::string& err);
    bool readVarBind(const uint8_t*& p, const uint8_t* end,
                     std::string& outOid, std::string& err);

#if 0
    // ============================================================
    // Encoding helpers (BER)
    // ============================================================
    void encodeLength(std::vector<uint8_t>& out, size_t len);
    std::vector<uint8_t> encodeInteger(int value);
    std::vector<uint8_t> encodeString(const std::string& value);
    std::vector<uint8_t> encodeOid(const std::string& oid);
    std::vector<uint8_t> encodeVarBind(const std::string& oid,
                                       const UpsParameterValue& value,
                                       UpsParameterType type);
#endif
};

} // namespace snmp

#endif // SNMP_CODEC_H
