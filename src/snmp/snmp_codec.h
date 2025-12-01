#ifndef SNMP_CODEC_H
#define SNMP_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

#include "ups_data_store.h"
#include "ups_types.h"

namespace snmp {

// ------------------------------------------------------------
// Parsed SNMP GET Request
// ------------------------------------------------------------
struct SnmpGetRequest {
    int requestId{0};
    std::vector<std::string> oids{};
};

// ------------------------------------------------------------
// SNMP Codec (ASN.1 BER encoder/decoder for SNMP v1)
// ------------------------------------------------------------
class SnmpCodec {
public:
    // Decode SNMP GET Request (true=success)
    bool decodeGetRequest(const uint8_t* data, size_t size, SnmpGetRequest& outRequest,
                          std::string* err = 0);

    // Encode SNMP GET Response
    std::vector<uint8_t> encodeGetResponse(int requestId, const std::vector<std::string>& oids,
                                           const UpsDataStore& store);

private:
    // Low-level ASN.1 BER helpers
    size_t decodeLength(const uint8_t*& p, const uint8_t* end);
    void encodeLength(std::vector<uint8_t>& out, size_t len);

    std::vector<uint8_t> encodeInteger(int value);
    std::vector<uint8_t> encodeString(const std::string& value);
    std::vector<uint8_t> encodeOid(const std::string& oidStr);

    std::vector<uint8_t> encodeVarBind(const std::string& oid, const UpsParameterValue& value,
                                       UpsParameterType type);

    std::vector<int> parseOidStr(const std::string& oid);
};

}  // namespace snmp

#endif  // SNMP_CODEC_H
