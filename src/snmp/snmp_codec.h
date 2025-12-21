/**
 * @file snmp_codec.h
 * @brief ASN.1 BER кодек для SNMP v1 и v2c.
 *
 * Содержит структуры и классы, отвечающие за:
 * - декодирование SNMP GET-запросов;
 * - формирование SNMP GET-RESPONSE сообщений;
 * - низкоуровневую работу с ASN.1 BER.
 *
 * Кодек не выполняет сетевых операций и не хранит состояние,
 * а используется SNMP-агентом как чистый компонент обработки данных.
 */
#ifndef SNMP_CODEC_H
#define SNMP_CODEC_H

#include <cstdint>
#include <string>
#include <vector>

#include "snmp_ber_writer.h"
#include "ups_data_store.h"
#include "ups_types.h"

/// @ingroup snmp

/**
 * @enum SnmpVersion
 * @brief Поддерживаемые версии протокола SNMP.
 */
enum class SnmpVersion : uint8_t {
    V_1 = 0,  ///< SNMP версии 1
    V_2C = 1  ///< SNMP версии 2c
};

/**
 * @struct SnmpGetRequest
 * @brief Представление разобранного SNMP GET-запроса.
 *
 * Используется как результат декодирования входного SNMP-запроса
 * и передаётся в кодек для формирования ответа.
 */
struct SnmpGetRequest {
    int requestId{ 0 };                       ///< Идентификатор SNMP-запроса.
    SnmpVersion version{ SnmpVersion::V_1 };  ///< Версия протокола SNMP.
    std::string community{};                  ///< Community string (например, "public").
    std::vector<Oid> oids;                    ///< Список OID из varbind-list.
};

/**
 * @class SnmpCodec
 * @brief Кодек SNMP-сообщений (ASN.1 BER).
 *
 * Класс отвечает за:
 * - декодирование SNMP GET-запросов (v1, v2c);
 * - формирование SNMP GET-RESPONSE сообщений;
 * - низкоуровневую работу с ASN.1 BER структурами.
 *
 * SnmpCodec не выполняет сетевых операций и не хранит состояние
 * между вызовами. Он используется SNMP-агентом как компонент
 * преобразования данных.
 */
class SnmpCodec {
public:
    /**
     * @brief Декодирует SNMP GET-запрос.
     *
     * @param data Указатель на входной буфер с SNMP-сообщением.
     * @param size Размер входного буфера в байтах.
     * @param outReq Структура для записи разобранного запроса.
     * @param err Указатель для записи сообщения об ошибке (опционально).
     *
     * @return true, если запрос успешно декодирован.
     */
    bool decodeGetRequest(const uint8_t* data,
                          size_t size,
                          SnmpGetRequest& outReq,
                          ErrorMessage* err = nullptr);

    /**
     * @brief Формирует SNMP GET-RESPONSE сообщение.
     *
     * @param req Исходный SNMP GET-запрос.
     * @param store Хранилище данных UPS.
     * @param out Буфер для сформированного SNMP-ответа.
     *
     * @return true, если ответ успешно сформирован.
     */
    bool encodeGetResponse(const SnmpGetRequest& req,
                           const UpsDataStore& store,
                           std::vector<uint8_t>& out);

private:
    /**
     * @brief ASN.1 BER теги, используемые при кодировании и декодировании SNMP.
     *
     * Константы определяют типы ASN.1 элементов и SNMP PDU, используемых
     * кодеком.
     */
    enum : uint8_t {
        TAG_SEQUENCE = 0x30,
        TAG_INTEGER = 0x02,
        TAG_OCTETSTRING = 0x04,
        TAG_NULL = 0x05,
        TAG_OID = 0x06,
        TAG_GETREQUEST = 0xA0,
        TAG_GETRESPONSE = 0xA2
    };

    /**
     * @brief Вспомогательные методы для декодирования ASN.1 BER.
     *
     * Набор низкоуровневых функций, используемых при разборе
     * SNMP GET-запроса. Каждый метод обрабатывает отдельный
     * элемент ASN.1 структуры и продвигает указатель входного буфера.
     *
     * Методы предназначены исключительно для внутреннего
     * использования кодеком.
     */

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

    /**
     * @brief Вспомогательные методы для кодирования ASN.1 BER.
     *
     * Набор низкоуровневых функций, используемых при формировании
     * SNMP GET-RESPONSE сообщения. Методы являются логическим
     * отражением функций декодирования.
     *
     * Предназначены исключительно для внутреннего использования
     * кодеком.
     */

    void encodeInteger(BerWriter& w, int value) const;
    void encodeOctetString(BerWriter& w, const std::string& str) const;
    void encodeNull(BerWriter& w) const;
    void encodeOid(BerWriter& w, const Oid& oid) const;

    /**
     * @brief Методы формирования структур SNMP GET-RESPONSE.
     *
     * Набор функций, отвечающих за построение SNMP-ответа
     * на основе разобранного GET-запроса и данных из UpsDataStore.
     *
     * Эти методы формируют логическую структуру ответа,
     * используя низкоуровневые ASN.1 BER кодеры.
     */

    void encodeVarBind(BerWriter& w, const Oid& oid, const UpsParameter* param) const;
    void encodeVarBindList(BerWriter& w,
                           const std::vector<Oid>& oids,
                           const UpsDataStore& store) const;
    void encodeGetResponsePdu(BerWriter& w,
                              const SnmpGetRequest& req,
                              const UpsDataStore& store) const;

    // Дружественный класс для доступа к приватным методам при тестировании
    friend class SnmpCodecTestAccess;
};

/**
 * @class SnmpCodecTestAccess
 * @brief Вспомогательный класс для тестирования приватных методов SnmpCodec.
 *
 * Используется исключительно в unit-тестах и предоставляет
 * контролируемый доступ к приватным методам кодека.
 */
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

#endif  // SNMP_CODEC_H
