#include "snmp/snmp_codec.h"
#include <cstdio>   // временно для отладки (потом уберём)


bool snmp::SnmpCodec::decodeGetRequest(const uint8_t* data, size_t size,
                                       SnmpGetRequest& outReq,
                                       std::string* errPtr)
{
    std::string err;
    const uint8_t* p   = data;
    const uint8_t* end = data + size;
    
    // 1) Top-level SEQUENCE (Message)
    const uint8_t* msgEnd = nullptr;
    if (!readSequence(p, end, msgEnd, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 2) version INTEGER
    int version = 0;
    if (!readInteger(p, msgEnd, version, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }
    if (version != 0) {
        if (errPtr) *errPtr = "Unsupported SNMP version (expected v1 = 0)";
        return false;
    }

    // 3) community (public) OCTET STRING
    if (!readOctetString(p, msgEnd, outReq.community, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 4) GetRequest PDU tag
    if (!readTag(p, msgEnd, TAG_GETREQUEST, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 5) PDU length (after A0 tag)
    size_t pduLen = 0;
    if (!readLength(p, msgEnd, pduLen, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }
    const uint8_t* pduEnd = p + pduLen;
    if (pduEnd > msgEnd) {
        if (errPtr) *errPtr = "PDU length exceeds message bounds";
        return false;
    }

    // 6) request-id INTEGER
    int requestId = 0;
    if (!readInteger(p, pduEnd, requestId, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }
    outReq.requestId = requestId;

    // 7) error-status INTEGER
    int errorStatus = 0;
    if (!readInteger(p, pduEnd, errorStatus, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 8) error-index INTEGER
    int errorIndex = 0;
    if (!readInteger(p, pduEnd, errorIndex, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 9) VarBindList SEQUENCE
    const uint8_t* vblEnd = nullptr;
    if (!readSequence(p, pduEnd, vblEnd, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 10) Parse varbinds
    while (p < vblEnd) {
        std::string oid;
        if (!readVarBind(p, vblEnd, oid, err)) {
            if (errPtr) *errPtr = err;
            return false;
        }
        outReq.oids.push_back(oid);
    }

    return true;
}





// ============================================================
// encodeGetResponse (SNMPv1)
// ============================================================
std::vector<uint8_t> snmp::SnmpCodec::encodeGetResponse(
        int requestId,
        const std::vector<std::string>& oids,
        const UpsDataStore& store)
{
    std::vector<uint8_t> resp;

    // ------------------------------------------------------------
    // 1) Message SEQUENCE (placeholder)
    // ------------------------------------------------------------
    resp.push_back(TAG_SEQUENCE);
    resp.push_back(0); // length placeholder, we will fill later
    size_t msgLenPos = resp.size() - 1;

    // ------------------------------------------------------------
    // 2) version = INTEGER(0)
    // ------------------------------------------------------------
    {
        std::vector<uint8_t> v = encodeInteger(0);
        resp.insert(resp.end(), v.begin(), v.end());
    }

    // ------------------------------------------------------------
    // 3) community string ("public" — as in request)
    // ------------------------------------------------------------
    {
        std::vector<uint8_t> s = encodeString("public");
        resp.insert(resp.end(), s.begin(), s.end());
    }

    // ------------------------------------------------------------
    // 4) GetResponse-PDU [A2]
    // ------------------------------------------------------------
    resp.push_back(TAG_GETRESPONSE);
    resp.push_back(0); // length placeholder
    size_t pduLenPos = resp.size() - 1;

    // PDU content starts here
    size_t pduStart = resp.size();

    // ------------------------------------------------------------
    // 5) request-id INTEGER
    // ------------------------------------------------------------
    {
        std::vector<uint8_t> v = encodeInteger(requestId);
        resp.insert(resp.end(), v.begin(), v.end());
    }

    // ------------------------------------------------------------
    // 6) error-status INTEGER (always 0)
    // ------------------------------------------------------------
    {
        std::vector<uint8_t> v = encodeInteger(0);
        resp.insert(resp.end(), v.begin(), v.end());
    }

    // ------------------------------------------------------------
    // 7) error-index INTEGER (always 0)
    // ------------------------------------------------------------
    {
        std::vector<uint8_t> v = encodeInteger(0);
        resp.insert(resp.end(), v.begin(), v.end());
    }

    // ------------------------------------------------------------
    // 8) VarBindList SEQUENCE
    // ------------------------------------------------------------
    resp.push_back(TAG_SEQUENCE);
    resp.push_back(0); // varbind-list length placeholder
    size_t vblLenPos = resp.size() - 1;

    size_t vblStart = resp.size();

    // ------------------------------------------------------------
    // 9) Build each VarBind
    // ------------------------------------------------------------
    for (const std::string& oid : oids) {

        const UpsParameter* p = store.get(oid);
        UpsParameterValue value;
        UpsParameterType type = UpsParameterType::String;

        bool hasValue = (p != nullptr);
        if (hasValue) {
            value = p->value;
            type = p->type;
        }

        std::vector<uint8_t> vb =
            encodeVarBind(oid, hasValue ? value : "", hasValue ? type : UpsParameterType::String);

        resp.insert(resp.end(), vb.begin(), vb.end());
    }

    // ------------------------------------------------------------
    // Fix lengths
    // ------------------------------------------------------------

    // varbind-list length
    size_t vblLen = resp.size() - vblStart;
    resp[vblLenPos] = static_cast<uint8_t>(vblLen);

    // PDU length
    size_t pduLen = resp.size() - pduStart;
    resp[pduLenPos] = static_cast<uint8_t>(pduLen);

    // message length
    size_t msgLen = resp.size() - 2; // total minus tag & length
    resp[msgLenPos] = static_cast<uint8_t>(msgLen);

    return resp;
}



// ============================================================
// ASN.1 Helpers
// ============================================================

// ------------------------------------------------------------
// readTag: читает и проверяет ASN.1 тег
// ------------------------------------------------------------
bool snmp::SnmpCodec::readTag(const uint8_t*& p, const uint8_t* end,
                              uint8_t expectedTag, std::string& err)
{
    if (p >= end) {
        err = "Unexpected end of buffer while reading tag";
        return false;
    }

    uint8_t tag = *p;
    if (tag != expectedTag) {
        char buf[128];
        std::snprintf(buf, sizeof(buf),
                      "Invalid tag: expected 0x%02X, got 0x%02X",
                      expectedTag, tag);
        err = buf;
        return false;
    }

    p++; // consume tag
    return true;
}

// ------------------------------------------------------------
// readLength: читает ASN.1 длину (short/long form)
// ------------------------------------------------------------
bool snmp::SnmpCodec::readLength(const uint8_t*& p, const uint8_t* end,
                                 size_t& outLen, std::string& err)
{
    if (p >= end) {
        err = "Unexpected end of buffer while reading length";
        return false;
    }

    uint8_t first = *p++;
    if (first < 0x80) {
        // short form
        outLen = first;
        return true;
    }

    // long form
    uint8_t count = first & 0x7F;
    if (count == 0) {
        err = "Invalid ASN.1 length: long form with zero count";
        return false;
    }
    if (p + count > end) {
        err = "Length field exceeds buffer";
        return false;
    }

    size_t len = 0;
    for (uint8_t i = 0; i < count; i++) {
        len = (len << 8) | (*p++);
    }

    outLen = len;
    return true;
}

// ------------------------------------------------------------
// readInteger: читает ASN.1 INTEGER
// ------------------------------------------------------------
bool snmp::SnmpCodec::readInteger(const uint8_t*& p, const uint8_t* end,
                                  int& outValue, std::string& err)
{
    if (!readTag(p, end, TAG_INTEGER, err))
        return false;

    size_t len = 0;
    if (!readLength(p, end, len, err))
        return false;

    if (p + len > end) {
        err = "INTEGER content exceeds buffer";
        return false;
    }

    int value = 0;
    for (size_t i = 0; i < len; i++) {
        value = (value << 8) | p[i];
    }

    p += len; // consume value
    outValue = value;
    return true;
}

// ------------------------------------------------------------
// readOctetString: читает OCTET STRING → outStr
// ------------------------------------------------------------
bool snmp::SnmpCodec::readOctetString(const uint8_t*& p, const uint8_t* end,
                                      std::string& outStr, std::string& err)
{
    if (!readTag(p, end, TAG_OCTETSTRING, err))
        return false;

    size_t len = 0;
    if (!readLength(p, end, len, err))
        return false;

    if (p + len > end) {
        err = "OCTET STRING exceeds buffer";
        return false;
    }

    outStr.assign(reinterpret_cast<const char*>(p), len);
    p += len;
    return true;
}

// ------------------------------------------------------------
// readSequence: читает SEQUENCE и возвращает seqEnd
// ------------------------------------------------------------
bool snmp::SnmpCodec::readSequence(const uint8_t*& p, const uint8_t* end,
                                   const uint8_t*& seqEnd, std::string& err)
{
    if (!readTag(p, end, TAG_SEQUENCE, err)) {
        return false;
    }

    size_t len = 0;
    if (!readLength(p, end, len, err)) {
        return false;
    }
    
    if (p + len > end) {
        err = "SEQUENCE length exceeds buffer";
        return false;
    }

    seqEnd = p + len;

    return true;
}

// ------------------------------------------------------------
// readOid: читает ASN.1 OID
// ------------------------------------------------------------
bool snmp::SnmpCodec::readOid(const uint8_t*& p, const uint8_t* end,
                              std::string& outOid, std::string& err)
{
    if (!readTag(p, end, TAG_OID, err))
        return false;

    size_t len = 0;
    if (!readLength(p, end, len, err))
        return false;

    if (p + len > end) {
        err = "OID content exceeds buffer";
        return false;
    }

    const uint8_t* oidEnd = p + len;

    if (len == 0) {
        err = "OID length is zero";
        return false;
    }

    // First byte encodes: (first * 40 + second)
    uint8_t fb = *p++;
    int first  = fb / 40;
    int second = fb % 40;

    outOid = std::to_string(first) + "." + std::to_string(second);

    int arc = 0;
    while (p < oidEnd) {
        uint8_t b = *p++;

        // Continuation bit
        arc = (arc << 7) | (b & 0x7F);

        if ((b & 0x80) == 0) {
            // arc finished
            outOid += "." + std::to_string(arc);
            arc = 0;
        }
    }

    if (arc != 0) {
        err = "OID ended unexpectedly (unfinished arc)";
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// readVarBind: читает один VarBind → только OID (значения игнорируем)
// ------------------------------------------------------------
bool snmp::SnmpCodec::readVarBind(const uint8_t*& p, const uint8_t* end,
                                  std::string& outOid, std::string& err)
{
    const uint8_t* vbEnd = nullptr;

    if (!readSequence(p, end, vbEnd, err))
        return false;

    if (!readOid(p, vbEnd, outOid, err))
        return false;

    // VALUE (we skip it)
    if (p >= vbEnd) {
        err = "VarBind missing value field";
        return false;
    }

    uint8_t tag = *p++;

    // Accept NULL, INTEGER, OCTET STRING — but we don't decode
    switch (tag) {
        case TAG_NULL:
        case TAG_INTEGER:
        case TAG_OCTETSTRING:
            break;

        default:
            err = "VarBind contains unsupported value type";
            return false;
    }

    // Read length and skip the value
    size_t len = 0;
    if (!readLength(p, vbEnd, len, err))
        return false;

    if (p + len > vbEnd) {
        err = "VarBind value exceeds VarBind end";
        return false;
    }

    p += len;
    return true;
}


// ============================================================
// ASN.1 Encoding helpers
// ============================================================

// ------------------------------------------------------------
// encodeLength: short form only (len < 128)
// SNMP packets rarely exceed this in UPS emulator.
// ------------------------------------------------------------
void snmp::SnmpCodec::encodeLength(std::vector<uint8_t>& out, size_t len)
{
    // For simplicity, we support only short form here (< 128).
    // If needed, long form can be added later.
    out.push_back(static_cast<uint8_t>(len));
}

// ------------------------------------------------------------
// encodeInteger (positive, fits in signed int)
// ------------------------------------------------------------
std::vector<uint8_t> snmp::SnmpCodec::encodeInteger(int value)
{
    std::vector<uint8_t> out;

    out.push_back(TAG_INTEGER);

    // Encode integer big-endian
    uint8_t buf[5]; // max 32-bit int + sign byte
    int idx = 0;

    int v = value;
    do {
        buf[idx++] = static_cast<uint8_t>(v & 0xFF);
        v >>= 8;
    } while (v != 0 && idx < 5);

    // Reverse to big-endian
    std::vector<uint8_t> val;
    for (int i = idx - 1; i >= 0; --i)
        val.push_back(buf[i]);

    encodeLength(out, val.size());
    out.insert(out.end(), val.begin(), val.end());

    return out;
}

// ------------------------------------------------------------
// encodeString (OCTET STRING)
// ------------------------------------------------------------
std::vector<uint8_t> snmp::SnmpCodec::encodeString(const std::string& value)
{
    std::vector<uint8_t> out;

    out.push_back(TAG_OCTETSTRING);
    encodeLength(out, value.size());

    out.insert(out.end(), value.begin(), value.end());
    return out;
}

// ------------------------------------------------------------
// encodeOid
//   "1.3.6.1.2.1.1.1.0"
// ------------------------------------------------------------
std::vector<uint8_t> snmp::SnmpCodec::encodeOid(const std::string& oid)
{
    std::vector<uint8_t> out;

    out.push_back(TAG_OID);

    // Split string by '.'
    std::vector<int> arcs;
    {
        size_t pos = 0;
        size_t next;
        while ((next = oid.find('.', pos)) != std::string::npos) {
            arcs.push_back(std::stoi(oid.substr(pos, next - pos)));
            pos = next + 1;
        }
        arcs.push_back(std::stoi(oid.substr(pos)));
    }

    if (arcs.size() < 2) {
        // minimally: 1.3.x
        // but we enforce correct SNMP format externally
        arcs.resize(2, 0);
    }

    std::vector<uint8_t> val;

    // first two arcs: (X * 40 + Y)
    val.push_back(static_cast<uint8_t>(arcs[0] * 40 + arcs[1]));

    // remaining arcs: base-128 encoding
    for (size_t i = 2; i < arcs.size(); i++) {
        int arc = arcs[i];

        // encode arc in base-128
        uint8_t tmp[10];
        int idx = 0;

        tmp[idx++] = arc & 0x7F;
        arc >>= 7;

        while (arc > 0) {
            tmp[idx++] = 0x80 | (arc & 0x7F);
            arc >>= 7;
        }

        // reverse
        for (int k = idx - 1; k >= 0; --k)
            val.push_back(tmp[k]);
    }

    encodeLength(out, val.size());
    out.insert(out.end(), val.begin(), val.end());

    return out;
}

// ------------------------------------------------------------
// encodeVarBind
// VarBind ::= SEQUENCE { OID, value }
// ------------------------------------------------------------
std::vector<uint8_t> snmp::SnmpCodec::encodeVarBind(
        const std::string& oid,
        const UpsParameterValue& value,
        UpsParameterType type)
{
    std::vector<uint8_t> out;

    // Start SEQUENCE
    out.push_back(TAG_SEQUENCE);
    out.push_back(0); // placeholder
    size_t lenPos = out.size() - 1;

    size_t start = out.size();

    // 1) OID
    {
        std::vector<uint8_t> v = encodeOid(oid);
        out.insert(out.end(), v.begin(), v.end());
    }

    // 2) Value
    if (value.empty()) {
        // Encode NULL
        out.push_back(TAG_NULL);
        out.push_back(0x00);
    } else {
        if (type == UpsParameterType::Integer) {
            std::vector<uint8_t> v = encodeInteger(std::stoi(value));
            out.insert(out.end(), v.begin(), v.end());
        } else {
            std::vector<uint8_t> v = encodeString(value);
            out.insert(out.end(), v.begin(), v.end());
        }
    }

    // Fix SEQUENCE length
    size_t len = out.size() - start;
    out[lenPos] = static_cast<uint8_t>(len);

    return out;
}
