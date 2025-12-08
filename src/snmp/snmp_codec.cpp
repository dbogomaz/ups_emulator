#include "snmp/snmp_codec.h"

#include <cstring>
#include <cstdio>  // временно для отладки (потом уберём)


#if 1 // public
// ============================================================
// decodeGetRequest (SNMPv1, SNMPv2c)
// ============================================================
bool snmp::SnmpCodec::decodeGetRequest(const uint8_t* data,
                                       size_t size,
                                       SnmpGetRequest& outReq,
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

// ============================================================
// encodeGetResponse (SNMPv1, SNMPv2c)
// ============================================================
bool snmp::SnmpCodec::encodeGetResponse(const SnmpGetRequest& req,
                                        const UpsDataStore& store,
                                        std::vector<uint8_t>& out) {
    out.clear();
    BerWriter w(out);

    // 1) Начинаем внешний SNMP message SEQUENCE
    size_t msgStart = w.beginSequence(TAG_SEQUENCE);

    // 2) Version SNMPv1 = 0, SNMPv2c = 1
    int ver = (req.version == SnmpVersion::V_1 ? 0 : 1);
    encodeInteger(w, ver);

    // 3) Community
    encodeOctetString(w, req.community);

    // 4) PDU
    encodeGetResponsePdu(w, req, store);

    // 5) Закрываем message SEQUENCE
    w.endSequence(msgStart);

    return true;
}

#endif

#if 1 // decoding helpers
// ============================================================
// ASN.1 decoding helpers
// ============================================================

// ------------------------------------------------------------
// readTagAndLength: читает тег expectedTag и длину (ASN.1)
// ------------------------------------------------------------
bool snmp::SnmpCodec::readTagAndLength(const uint8_t*& p, 
                                       const uint8_t* end, 
                                       uint8_t expectedTag, 
                                       size_t& outLen, 
                                       ErrorMessage& err) {
    // 1) Tag
    if (p >= end) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Unexpected end of buffer while reading tag 0x%02X",
                      expectedTag);
        err = buf;
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
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Unexpected end of buffer while reading tag 0x%02X length",
                      expectedTag);
        err = buf;
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
bool snmp::SnmpCodec::readSequence(const uint8_t*& p,
                                   const uint8_t* end,
                                   const uint8_t*& seqEnd,
                                   ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_SEQUENCE, len, err)) return false;

    seqEnd = p + len;

    return true;
}

// ------------------------------------------------------------
// readInteger: читает ASN.1 INTEGER
// ------------------------------------------------------------
bool snmp::SnmpCodec::readInteger(const uint8_t*& p,
                                  const uint8_t* end,
                                  int& outValue,
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
bool snmp::SnmpCodec::readOctetString(const uint8_t*& p,
                                      const uint8_t* end,
                                      std::string& outStr,
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
bool snmp::SnmpCodec::readGetRequestPdu(const uint8_t*& p,
                                        const uint8_t* end,
                                        const uint8_t*& pduEnd,
                                        ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_GETREQUEST, len, err)) return false;

    pduEnd = p + len;
    return true;
}

// ------------------------------------------------------------
// readOid: читает ASN.1 OID
// ------------------------------------------------------------
bool snmp::SnmpCodec::readOid(const uint8_t*& p,
                              const uint8_t* end,
                              Oid& outOid,
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
bool snmp::SnmpCodec::readVarBind(const uint8_t*& p,
                                  const uint8_t* end,
                                  Oid& outOid,
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

    p += valueLen;  // всё, VarBind закончился
    return true;
}
#endif

#if 1  // Encoding helpers
// ============================================================
// ASN.1 Encoding helpers
// ============================================================

// ============================================================
// Low-level ASN.1 encoding helpers
// ============================================================

void snmp::SnmpCodec::encodeInteger(BerWriter& w, int value) const {
    // Шаг 1: представляем число в виде 4 bytes big-endian
    uint8_t raw[4];
    raw[0] = (value >> 24) & 0xFF;
    raw[1] = (value >> 16) & 0xFF;
    raw[2] = (value >> 8) & 0xFF;
    raw[3] = value & 0xFF;

    // Шаг 2: удаляем лишние leading bytes
    int start = 0;
    while (start < 3) {
        // положительное число - удаляем 00, если следующий байт < 0x80
        if (raw[start] == 0x00 && (raw[start + 1] & 0x80) == 0) {
            start++;
            continue;
        }
        // положительное число - удаляем 00, если следующий байт < 0x80
        if (raw[start] == 0xFF && (raw[start + 1] & 0x80) == 0x80) {
            start++;
            continue;
        }
        break;
    }
    const uint8_t* encoded = raw + start;
    size_t n = 4 - start;

    // Шаг 3: ASN.1 INTEGER
    w.putTag(TAG_INTEGER);
    w.putLength(n);
    w.putBytes(encoded, n);
}

void snmp::SnmpCodec::encodeOctetString(BerWriter& w, const std::string& str) const {
    w.putTag(TAG_OCTETSTRING);
    w.putLength(str.size());
    if (!str.empty()) w.putBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

void snmp::SnmpCodec::encodeNull(BerWriter& w) const {
    w.putTag(TAG_NULL);  // 0x05
    w.putLength(0);      // длина = 0
}

void snmp::SnmpCodec::encodeOid(BerWriter& w, const Oid& oidStr) const {
    // ------------------------------------------------------------
    // 1. Парсим OID "1.3.6.1.4.1.9999.1" -> [1,3,6,1,4,1,9999,1]
    // ------------------------------------------------------------
    std::vector<uint32_t> parts;
    size_t pos = 0;
    while (pos < oidStr.size()) {
        size_t dot = oidStr.find('.', pos);
        std::string token =
            (dot == std::string::npos) ? oidStr.substr(pos) : oidStr.substr(pos, dot - pos);
        parts.push_back(std::stoul(token));
        if (dot == std::string::npos) break;
        pos = dot + 1;
    }
    if (parts.size() < 2) {
        w.putTag(TAG_OID);
        w.putLength(0);
        return;
    }

    // ------------------------------------------------------------
    // 2. ASN.1 правило: первый байт = 40 * X + Y
    // ------------------------------------------------------------
    uint8_t first = uint8_t(parts[0] * 40 + parts[1]);
    std::vector<uint8_t> enc;
    enc.push_back(first);

    // ------------------------------------------------------------
    // Функция минимального BER base-128 кодирования
    // ------------------------------------------------------------
    auto encodeBase128 = [&](uint32_t value) {
        uint8_t tmp[10];
        int count = 0;
        do {
            tmp[count++] = value & 0x7F;
            value >>= 7;
        } while (value > 0);
        for (int i = count - 1; i >= 0; --i) {
            uint8_t b = tmp[i];
            if (i != 0) b |= 0x80;  // continuation bit
            enc.push_back(b);
        }
    };

    // ------------------------------------------------------------
    // 3. Кодируем все компоненты, начиная с третьего
    // ------------------------------------------------------------
    for (size_t i = 2; i < parts.size(); ++i) {
        encodeBase128(parts[i]);
    }

    // ------------------------------------------------------------
    // 4. Собираем TLV (TAG_OID | LENGTH | VALUE)
    // ------------------------------------------------------------
    w.putTag(TAG_OID);
    w.putLength(enc.size());
    w.putBytes(enc.data(), enc.size());
}

// ============================================================
// SNMP structural encoders
// ============================================================

void snmp::SnmpCodec::encodeVarBind(BerWriter& w, const Oid& oid, const UpsParameter* param) const {
    // -------------------------------
    // 1. Начинаем VarBind (SEQUENCE)
    // -------------------------------
    size_t seqStart = w.beginSequence(TAG_SEQUENCE);

    // -------------------------------
    // 2. OID
    // -------------------------------
    encodeOid(w, oid);

    // -------------------------------
    // 3. Значение
    // -------------------------------
    if (!param) {
        // Нет значения - ASN.1 NULL
        encodeNull(w);
    } else {
        switch (param->type) {
            case UpsParameterType::Integer:
                encodeInteger(w, std::stoi(param->value));
                break;

            case UpsParameterType::String:
                encodeOctetString(w, param->value);
                break;

            default:
                // если тип пока не реализован - NULL
                encodeNull(w);
                break;
        }
    }

    // -------------------------------
    // 4. Завершаем SEQUENCE
    // -------------------------------
    w.endSequence(seqStart);
}

void snmp::SnmpCodec::encodeVarBindList(BerWriter& w, const std::vector<Oid>& oids,
                                        const UpsDataStore& store) const {
    // Начинаем SEQUENCE OF VarBind
    size_t seqStart = w.beginSequence(TAG_SEQUENCE);

    // Каждый VarBind = SEQUENCE { OID, Value }
    for (const auto& oid : oids) {
        const UpsParameter* param = nullptr;
        if (store.has(oid)) {
            param = store.get(oid);
        }
        encodeVarBind(w, oid, param);
    }

    // Завершаем SEQUENCE OF
    w.endSequence(seqStart);
}

void snmp::SnmpCodec::encodeGetResponsePdu(BerWriter& w, const SnmpGetRequest& req,
                                           const UpsDataStore& store) const {
    // Begin PDU (tag = A2)
    size_t pduStart = w.beginSequence(TAG_GETRESPONSE);
    encodeInteger(w, req.requestId);// request-id
    encodeInteger(w, 0); // error-status = 0
    encodeInteger(w, 0); // error-index = 0
    encodeVarBindList(w, req.oids, store); // VarBindList
    // close PDU sequence
    w.endSequence(pduStart);
}

#endif
