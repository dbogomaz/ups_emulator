#include "snmp_codec.h"

#include <cstdio>
#include <cstdlib>

namespace snmp {

// ------------------------------------------------------------
// Decode ASN.1 BER length field
// ------------------------------------------------------------
size_t SnmpCodec::decodeLength(const uint8_t*& p, const uint8_t* end) {
    if (p >= end) return 0;

    uint8_t first = *p++;
    if (first < 0x80) {
        return first;
    }

    int count = first & 0x7F;
    size_t len = 0;
    while (count-- > 0 && p < end) {
        len = (len << 8) | *p++;
    }
    return len;
}

// ------------------------------------------------------------
// Encode ASN.1 BER length field
// ------------------------------------------------------------
void SnmpCodec::encodeLength(std::vector<uint8_t>& out, size_t len) {
    if (len < 0x80) {
        out.push_back((uint8_t)len);
        return;
    }

    uint8_t buf[8];
    int count = 0;

    size_t tmp = len;
    while (tmp > 0) {
        buf[count++] = tmp & 0xFF;
        tmp >>= 8;
    }

    out.push_back(0x80 | count);
    for (int i = count - 1; i >= 0; --i) out.push_back(buf[i]);
}

// ------------------------------------------------------------
// Parse OID string into integer arcs
// ------------------------------------------------------------
std::vector<int> SnmpCodec::parseOidStr(const std::string& oid) {
    std::vector<int> parts;
    size_t start = 0;

    while (start < oid.size()) {
        size_t pos = oid.find('.', start);
        if (pos == std::string::npos) pos = oid.size();
        parts.push_back(std::atoi(oid.substr(start, pos - start).c_str()));
        start = pos + 1;
    }

    return parts;
}

// ------------------------------------------------------------
// Encode ASN.1 OBJECT IDENTIFIER
// ------------------------------------------------------------
std::vector<uint8_t> SnmpCodec::encodeOid(const std::string& oidStr) {
    std::vector<int> parts = parseOidStr(oidStr);
    std::vector<uint8_t> body;

    if (parts.size() < 2) {
        // Encode empty OID as 0.0
        return {0x06, 1, 0};
    }

    // First byte = 40 * X + Y
    body.push_back((uint8_t)(parts[0] * 40 + parts[1]));

    // Remaining arcs in base-128
    for (size_t i = 2; i < parts.size(); ++i) {
        int v = parts[i];
        uint8_t buf[8];
        int count = 0;

        do {
            buf[count++] = v & 0x7F;
            v >>= 7;
        } while (v > 0);

        for (int j = count - 1; j > 0; --j) body.push_back(buf[j] | 0x80);

        body.push_back(buf[0]);
    }

    std::vector<uint8_t> out;
    out.push_back(0x06);
    encodeLength(out, body.size());
    out.insert(out.end(), body.begin(), body.end());
    return out;
}

// ------------------------------------------------------------
// Encode ASN.1 INTEGER
// ------------------------------------------------------------
std::vector<uint8_t> SnmpCodec::encodeInteger(int value) {
    std::vector<uint8_t> body;

    if (value >= -128 && value <= 127) {
        body.push_back((uint8_t)value);
    } else if (value >= -32768 && value <= 32767) {
        body.push_back((uint8_t)((value >> 8) & 0xFF));
        body.push_back((uint8_t)(value & 0xFF));
    } else {
        body.push_back((uint8_t)((value >> 24) & 0xFF));
        body.push_back((uint8_t)((value >> 16) & 0xFF));
        body.push_back((uint8_t)((value >> 8) & 0xFF));
        body.push_back((uint8_t)(value & 0xFF));
    }

    std::vector<uint8_t> out;
    out.push_back(0x02);  // INTEGER tag
    encodeLength(out, body.size());
    out.insert(out.end(), body.begin(), body.end());
    return out;
}

// ------------------------------------------------------------
// Encode ASN.1 OCTET STRING
// ------------------------------------------------------------
std::vector<uint8_t> SnmpCodec::encodeString(const std::string& value) {
    std::vector<uint8_t> out;
    out.push_back(0x04);
    encodeLength(out, value.size());
    out.insert(out.end(), value.begin(), value.end());
    return out;
}

// ------------------------------------------------------------
// Encode VarBind
// ------------------------------------------------------------
std::vector<uint8_t> SnmpCodec::encodeVarBind(const std::string& oid,
                                              const UpsParameterValue& value,
                                              UpsParameterType type) {
    std::vector<uint8_t> vb;

    // Encode OID
    std::vector<uint8_t> oidEnc = encodeOid(oid);
    vb.insert(vb.end(), oidEnc.begin(), oidEnc.end());

    // Encode value depending on type
    if (type == UpsParameterType::Integer) {
        int v = std::atoi(value.c_str());
        std::vector<uint8_t> enc = encodeInteger(v);
        vb.insert(vb.end(), enc.begin(), enc.end());
    } else {
        // String or anything else
        std::vector<uint8_t> enc = encodeString(value);
        vb.insert(vb.end(), enc.begin(), enc.end());
    }

    // Wrap into SEQUENCE
    std::vector<uint8_t> out;
    out.push_back(0x30);
    encodeLength(out, vb.size());
    out.insert(out.end(), vb.begin(), vb.end());
    return out;
}

// ------------------------------------------------------------
// Decode SNMP GET Request
// ------------------------------------------------------------
bool SnmpCodec::decodeGetRequest(const uint8_t* data, size_t size, SnmpGetRequest& outReq,
                                 std::string* err) {
    const uint8_t* p = data;
    const uint8_t* end = data + size;

    // Top-level SEQUENCE
    if (p >= end || *p++ != 0x30) {
        if (err) *err = "Not a SEQUENCE";
        return false;
    }

    size_t seqLen = decodeLength(p, end);
    if (p + seqLen > end) {
        if (err) *err = "Invalid SEQUENCE length";
        return false;
    }

    // Version INTEGER
    if (*p++ != 0x02) return false;
    size_t verLen = decodeLength(p, end);
    int version = 0;

    while (verLen-- > 0) version = (version << 8) | *p++;

    if (version != 0) {
        if (err) *err = "Only SNMPv1 supported";
        return false;
    }

    // Community
    if (*p++ != 0x04) return false;
    size_t commLen = decodeLength(p, end);
    p += commLen;

    // PDU tag A0 = GetRequest-PDU
    if (*p++ != 0xA0) {
        if (err) *err = "Expected GetRequest (A0)";
        return false;
    }

    // request-id
    if (*p++ != 0x02) return false;
    size_t idLen = decodeLength(p, end);

    int requestId = 0;
    while (idLen-- > 0) requestId = (requestId << 8) | *p++;

    outReq.requestId = requestId;

    // error-status
    if (*p++ != 0x02) return false;
    p += decodeLength(p, end) + 1;

    // error-index
    if (*p++ != 0x02) return false;
    p += decodeLength(p, end) + 1;

    // VarBindList
    if (*p++ != 0x30) return false;
    size_t vblLen = decodeLength(p, end);
    const uint8_t* vblEnd = p + vblLen;

    // Parse each VarBind
    while (p < vblEnd) {
        if (*p++ != 0x30) return false;
        size_t vbLen = decodeLength(p, end);
        const uint8_t* vbEnd = p + vbLen;

        // OID
        if (*p++ != 0x06) return false;
        size_t oidLen = decodeLength(p, end);

        const uint8_t* oidPtr = p;
        p += oidLen;

        // Decode OID arcs
        std::string oid;
        int firstByte = *oidPtr++;
        int first = firstByte / 40;
        int second = firstByte % 40;

        oid = std::to_string(first) + "." + std::to_string(second);

        int arc = 0;
        while (oidPtr < p) {
            uint8_t b = *oidPtr++;
            if (b & 0x80)
                arc = (arc << 7) | (b & 0x7F);
            else {
                arc = (arc << 7) | b;
                oid += "." + std::to_string(arc);
                arc = 0;
            }
        }

        outReq.oids.push_back(oid);

        // Skip value
        p = vbEnd;
    }

    return true;
}

// ------------------------------------------------------------
// Encode SNMP GET RESPONSE PDU
// ------------------------------------------------------------
std::vector<uint8_t> SnmpCodec::encodeGetResponse(int requestId,
                                                  const std::vector<std::string>& oids,
                                                  const UpsDataStore& store) {
    std::vector<uint8_t> pdu;

    // request-id
    std::vector<uint8_t> rid = encodeInteger(requestId);
    pdu.insert(pdu.end(), rid.begin(), rid.end());

    // error-status = 0
    std::vector<uint8_t> err = encodeInteger(0);
    pdu.insert(pdu.end(), err.begin(), err.end());

    // error-index = 0
    std::vector<uint8_t> idx = encodeInteger(0);
    pdu.insert(pdu.end(), idx.begin(), idx.end());

    // VarBindList
    std::vector<uint8_t> vbl;

    for (size_t i = 0; i < oids.size(); ++i) {
        const UpsParameter* p = store.get(oids[i]);

        if (p) {
            // normal varbind
            std::vector<uint8_t> vb = encodeVarBind(p->oid, p->value, p->type);
            vbl.insert(vbl.end(), vb.begin(), vb.end());
        } else {
            // OID not found → NULL (05 00)
            std::vector<uint8_t> oidEnc = encodeOid(oids[i]);
            std::vector<uint8_t> vb;

            vb.insert(vb.end(), oidEnc.begin(), oidEnc.end());
            vb.push_back(0x05);  // NULL
            vb.push_back(0x00);

            std::vector<uint8_t> seq;
            seq.push_back(0x30);
            encodeLength(seq, vb.size());
            seq.insert(seq.end(), vb.begin(), vb.end());

            vbl.insert(vbl.end(), seq.begin(), seq.end());
        }
    }

    // wrap vbl
    std::vector<uint8_t> vblSeq;
    vblSeq.push_back(0x30);
    encodeLength(vblSeq, vbl.size());
    vblSeq.insert(vblSeq.end(), vbl.begin(), vbl.end());

    pdu.insert(pdu.end(), vblSeq.begin(), vblSeq.end());

    // Wrap as GetResponse (A2)
    std::vector<uint8_t> resp;
    resp.push_back(0xA2);
    encodeLength(resp, pdu.size());
    resp.insert(resp.end(), pdu.begin(), pdu.end());

    // Full SNMP message
    std::vector<uint8_t> out;

    out.push_back(0x30);  // SEQUENCE start
    std::vector<uint8_t> inner;

    // version = 0
    std::vector<uint8_t> ver = encodeInteger(0);
    inner.insert(inner.end(), ver.begin(), ver.end());

    // community = "public"
    const char* comm = "public";
    std::vector<uint8_t> commEnc;
    commEnc.push_back(0x04);
    encodeLength(commEnc, 6);
    commEnc.insert(commEnc.end(), comm, comm + 6);
    inner.insert(inner.end(), commEnc.begin(), commEnc.end());

    // PDU
    inner.insert(inner.end(), resp.begin(), resp.end());

    // wrap SEQUENCE
    out.push_back(0x30);
    encodeLength(out, inner.size());
    out.insert(out.end(), inner.begin(), inner.end());

    return out;
}

}  // namespace snmp
