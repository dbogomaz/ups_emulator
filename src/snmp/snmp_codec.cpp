#include "snmp/snmp_codec.h"

#include <cstdio>  // временно для отладки (потом уберём)

// ============================================================
// decodeGetRequest (SNMPv1)
// ============================================================
bool snmp::SnmpCodec::decodeGetRequest(const uint8_t* data, size_t size, SnmpGetRequest& outReq,
                                       ErrorMessage* errPtr) {
    std::string err;
    const uint8_t* p = data;
    const uint8_t* end = data + size;

    // 1) Top-level SEQUENCE (Message)
    const uint8_t* msgEnd = nullptr;
    if (!readSequence(p, end, msgEnd, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 2) version INTEGER
    int verInt = 0;
    if (!readInteger(p, msgEnd, verInt, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }
    if (verInt != 0 && 
        verInt != 1) {  // поддерживаем v1(0) и v2c(1)
        if (errPtr) *errPtr = "Unsupported SNMP version (expected v1 = 0 or v2c = 1)";
        return false;
    }
    outReq.version = static_cast<SnmpVersion>(verInt);

    // 3) community (public) OCTET STRING
    if (!readOctetString(p, msgEnd, outReq.community, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 4) GetRequest PDU tag
    const uint8_t* pduEnd = nullptr;
    if (!readGetRequestPdu(p, msgEnd, pduEnd, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 5) request-id INTEGER
    int requestId = 0;
    if (!readInteger(p, pduEnd, requestId, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }
    outReq.requestId = requestId;

    // 6) error-status INTEGER
    int errorStatus = 0;
    if (!readInteger(p, pduEnd, errorStatus, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 7) error-index INTEGER
    int errorIndex = 0;
    if (!readInteger(p, pduEnd, errorIndex, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 8) VarBindList SEQUENCE
    const uint8_t* vblEnd = nullptr;
    if (!readSequence(p, pduEnd, vblEnd, err)) {
        if (errPtr) *errPtr = err;
        return false;
    }

    // 9) Parse varbinds
    while (p < vblEnd) {
        Oid oid;
        if (!readVarBind(p, vblEnd, oid, err)) {
            if (errPtr) *errPtr = err;
            return false;
        }
        outReq.oids.push_back(oid);
    }

    return true;
}

#if 0
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
#endif

// ============================================================
// ASN.1 decoding helpers
// ============================================================

// ------------------------------------------------------------
// readTagAndLength: читает тег expectedTag и длину (ASN.1)
// ------------------------------------------------------------
bool snmp::SnmpCodec::readTagAndLength(const uint8_t*& p, const uint8_t* end, uint8_t expectedTag,
                                       size_t& outLen, ErrorMessage& err) {
    // 1) Tag
    if (p >= end) {
        err = "Unexpected end of buffer while reading tag";
        return false;
    }
    uint8_t tag = *p;
    if (tag != expectedTag) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Invalid tag: expected 0x%02X, got 0x%02X", expectedTag,
                      tag);
        err = buf;
        return false;
    }
    p++;  // следующая должна быть длина

    // 2) Length
    if (p >= end) {
        err = "Unexpected end of buffer while reading length";
        return false;
    }

    // читаем маны стандарта ASN.1
    // length может быть в короткой или длинной форме
    // короткая форма: один байт, если < 128
    // длинная форма: первый байт: бит 8 установлен, биты 7-1: количество следующих байт длины

    uint8_t first = *p++;  // первый байт длины и указатель смещаем на следующий байт

    // Вспомогательная лямбда для проверки выхода за границы буфера
    auto check_bounds = [&](size_t len) -> bool {
        if (p + len > end) {
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Tag 0x%02X length exceeds buffer", expectedTag);
            err = buf;
            return false;
        }
        return true;
    };

    // 2.1) Short form
    if (first < 0x80) {
        outLen = first;  // длина в одном байте
        return check_bounds(outLen);
    }

    // 2.2) Long form
    uint8_t count = first & 0x7F;  // количество байт длины
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
    outLen = len;  // длина в нескольких байтах
    return check_bounds(outLen);
}

// ------------------------------------------------------------
// readSequence: читает SEQUENCE и возвращает seqEnd
// ------------------------------------------------------------
bool snmp::SnmpCodec::readSequence(const uint8_t*& p, const uint8_t* end, const uint8_t*& seqEnd,
                                   ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_SEQUENCE, len, err)) return false;

    seqEnd = p + len;

    return true;
}

// ------------------------------------------------------------
// readInteger: читает ASN.1 INTEGER
// ------------------------------------------------------------
bool snmp::SnmpCodec::readInteger(const uint8_t*& p, const uint8_t* end, int& outValue,
                                  ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_INTEGER, len, err)) return false;

    if (len == 0) {
        err = "Tag 0x02 length is zero";
        return false;
    }

    int value = 0;
    for (size_t i = 0; i < len; i++) {
        value = (value << 8) | p[i];
    }

    p += len;  // не забываем сдвинуть указатель
    outValue = value;
    return true;
}

// ------------------------------------------------------------
// readOctetString: читает OCTET STRING возвращает outStr
// ------------------------------------------------------------
bool snmp::SnmpCodec::readOctetString(const uint8_t*& p, const uint8_t* end, std::string& outStr,
                                      ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_OCTETSTRING, len, err)) return false;

    outStr.assign(reinterpret_cast<const char*>(p), len);
    p += len;
    return true;
}

// ------------------------------------------------------------
// readSequence: читает PDU и возвращает pduEnd
// ------------------------------------------------------------
bool snmp::SnmpCodec::readGetRequestPdu(const uint8_t*& p, const uint8_t* end,
                                        const uint8_t*& pduEnd, ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_GETREQUEST, len, err)) return false;

    pduEnd = p + len;
    return true;
}

// ------------------------------------------------------------
// readOid: читает ASN.1 OID
// ------------------------------------------------------------
bool snmp::SnmpCodec::readOid(const uint8_t*& p, const uint8_t* end, Oid& outOid,
                              ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_OID, len, err)) return false;

    if (len == 0) {
        err = "Tag 0x06 length is zero";
        return false;
    }

    const uint8_t* oidEnd = p + len;

    // OID состоит из чисел, например: 1.3.6.1.2
    // Первые два числа кодируются вместе в один байт по формуле:
    // first * 40 + second
    // Декодируем первый байт
    uint8_t fb = *p++;
    int first = fb / 40;
    int second = fb % 40;

    outOid = std::to_string(first) + "." + std::to_string(second);

    // Последующие байты: base-128 кодированные части OID
    // могут быть короткие (один байт) или длинные (несколько байт с битом продолжения)
    int arc = 0;
    while (p < oidEnd) {
        uint8_t b = *p++;

        // Бит продолжения
        arc = (arc << 7) | (b & 0x7F);

        if ((b & 0x80) == 0) {  // 0x80 - бит продолжения длинного числа
            // часть завершена
            outOid += "." + std::to_string(arc);
            arc = 0;
        }
    }

    // Проверка на незавершённую часть
    if (arc != 0) {
        err = "OID ended unexpectedly (unfinished arc)";
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// readVarBind: читает один VarBind - только OID (значения игнорируем)
// ------------------------------------------------------------
bool snmp::SnmpCodec::readVarBind(const uint8_t*& p, const uint8_t* end, Oid& outOid,
                                  ErrorMessage& err) {
    // 1) Внутренний SEQUENCE VarBind
    const uint8_t* vbEnd = nullptr;
    if (!readSequence(p, end, vbEnd, err)) return false;

    // 2) OID
    if (!readOid(p, vbEnd, outOid, err)) return false;

    // 3) VALUE — читаем через readTagAndLength
    uint8_t valueTag = 0;
    size_t valueLen = 0;

    if (p >= vbEnd) {
        err = "VarBind missing value field";
        return false;
    }

    valueTag = *p;  // читаем тег значения

    switch (valueTag) {
        case TAG_NULL:
        case TAG_INTEGER:
        case TAG_OCTETSTRING:
            break;
        default:
            err = "VarBind contains unsupported value type";
            return false;
    }

    // Теперь читаем Tag + Length
    if (!readTagAndLength(p, vbEnd, valueTag, valueLen, err)) return false;

    if (p + valueLen > vbEnd) {
        err = "VarBind value exceeds VarBind end";
        return false;
    }

    p += valueLen;  // всё, VarBind закончился
    return true;
}

#if 0
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
#endif
