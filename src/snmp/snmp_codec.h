#ifndef SNMP_CODEC_H
#define SNMP_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

#include "snmp_ber_writer.h"
#include "ups_data_store.h"
#include "ups_types.h"

namespace snmp {

// ------------------------------------------------------------
// SNMP Version
// ------------------------------------------------------------
enum class SnmpVersion : uint8_t { V_1 = 0, V_2C = 1 };

// ------------------------------------------------------------
// Parsed SNMP GET Request (SNMPv1, v2c)
// ------------------------------------------------------------
struct SnmpGetRequest {
    int requestId{ 0 };                       // request-id
    SnmpVersion version{ SnmpVersion::V_1 };  // SNMP version
    std::string community{};                  // community string ("public")
    std::vector<Oid> oids;                    // list of OIDs from varbind-list
};

// ------------------------------------------------------------
// Класс - обертка для тестирования приватных методов
// ------------------------------------------------------------
class SnmpCodecTestAccess;

// ------------------------------------------------------------
// SNMP Codec (ASN.1 BER encoder/decoder for SNMP v1, v2c)
// ------------------------------------------------------------
class SnmpCodec {
public:
    // Decode SNMP GET Request (SNMP v1, v2c)
    bool decodeGetRequest(const uint8_t* data,
                          size_t size,
                          SnmpGetRequest& outReq,
                          ErrorMessage* err = nullptr);

    // Encode SNMP GET-RESPONSE (SNMP v1/v2c)
    bool encodeGetResponse(const SnmpGetRequest& req,
                           const UpsDataStore& store,
                           std::vector<uint8_t>& out);

private:
    // ============================================================
    // ASN.1 TAG DEFINITIONS
    // ============================================================
    enum : uint8_t {
        TAG_SEQUENCE = 0x30,
        TAG_INTEGER = 0x02,
        TAG_OCTETSTRING = 0x04,
        TAG_NULL = 0x05,
        TAG_OID = 0x06,
        TAG_GETREQUEST = 0xA0,
        TAG_GETRESPONSE = 0xA2
    };

    // ============================================================
    // Low-level ASN.1 decoding helpers
    // ============================================================
    bool readTagAndLength(const uint8_t*& p,
                          const uint8_t* end,
                          uint8_t expectedTag,
                          size_t& outLen,
                          ErrorMessage& err);

    bool readSequence(const uint8_t*& p,
                      const uint8_t* end,
                      const uint8_t*& seqEnd,
                      ErrorMessage& err);
    bool readInteger(const uint8_t*& p, const uint8_t* end, int& outValue, ErrorMessage& err);
    bool readOctetString(const uint8_t*& p,
                         const uint8_t* end,
                         std::string& outStr,
                         ErrorMessage& err);
    bool readGetRequestPdu(const uint8_t*& p,
                           const uint8_t* end,
                           const uint8_t*& pduEnd,
                           ErrorMessage& err);
    bool readOid(const uint8_t*& p, const uint8_t* end, Oid& outOid, ErrorMessage& err);
    bool readVarBind(const uint8_t*& p, const uint8_t* end, Oid& outOid, ErrorMessage& err);

    // ============================================================
    // Low-level ASN.1 encoding helpers (mirror of decoding)
    // ============================================================
    void encodeInteger(BerWriter& w, int value) const;
    void encodeOctetString(BerWriter& w, const std::string& str) const;
    void encodeNull(BerWriter& w) const;
    void encodeOid(BerWriter& w, const Oid& oid) const;

    // ============================================================
    // SNMP structural encoders
    // ============================================================
    void encodeVarBind(BerWriter& w, const Oid& oid, const UpsParameter* param) const;
    void encodeVarBindList(BerWriter& w,
                           const std::vector<Oid>& oids,
                           const UpsDataStore& store) const;
    void encodeGetResponsePdu(BerWriter& w,
                              const SnmpGetRequest& req,
                              const UpsDataStore& store) const;
    // friend класс для тестирования
    friend class SnmpCodecTestAccess;
};

class SnmpCodecTestAccess {
public:
    static void encodeInteger(SnmpCodec& c, BerWriter& w, int v) { c.encodeInteger(w, v); }
    static void encodeOctetString(SnmpCodec& c, BerWriter& w, const std::string& s) {
        c.encodeOctetString(w, s);
    }
    static void encodeNull(SnmpCodec& c, BerWriter& w) { c.encodeNull(w); }
    static void encodeOid(SnmpCodec& c, BerWriter& w, const Oid& oid) { c.encodeOid(w, oid); }
    static void encodeVarBind(SnmpCodec& c, BerWriter& w, const Oid& oid, const UpsParameter* p) {
        c.encodeVarBind(w, oid, p);
    }
    static void encodeVarBindList(SnmpCodec& c,
                                  BerWriter& w,
                                  const std::vector<Oid>& oids,
                                  const UpsDataStore& store) {
        c.encodeVarBindList(w, oids, store);
    }
    static void encodeGetResponsePdu(SnmpCodec& c,
                                     BerWriter& w,
                                     const SnmpGetRequest& req,
                                     const UpsDataStore& store) {
        c.encodeGetResponsePdu(w, req, store);
    }
};

}  // namespace snmp

#endif  // SNMP_CODEC_H
