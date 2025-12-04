#ifndef SNMP_BER_WRITER_H
#define SNMP_BER_WRITER_H

#include <cstddef>
#include <cstdint>
#include <vector>

// ------------------------------------------------------------
// BerWriter — низкоуровневый ASN.1 BER-encoder
//
// Класс предназначен для сборки TLV-структур BER:
//   - запись Tag
//   - запись Length (short/long form)
//   - запись Value
//   - поддержка вложенных SEQUENCE/Constructed типов
//
// Используется SNMP encoder-ом для построения пакетов.
// ------------------------------------------------------------

class BerWriter {
public:
    // Создаёт writer, который пишет в переданный буфер.
    explicit BerWriter(std::vector<uint8_t>& out);

    // --------------------------------------------------------
    // Низкоуровневые записи
    // --------------------------------------------------------

    // Записать тег (например INTEGER = 0x02, SEQUENCE = 0x30)
    void putTag(uint8_t tag);

    // Записать длину в формате ASN.1 BER (short/long)
    void putLength(std::size_t len);

    // Записать произвольные байты
    void putBytes(const uint8_t* data, std::size_t len);

    // Записать один байт
    void putByte(uint8_t b);

    // --------------------------------------------------------
    // Работа с SEQUENCE / Constructed TLV
    // --------------------------------------------------------

    // Начать SEQUENCE или другой constructed тип.
    // Пишет:
    //   TAG
    //   placeholder для длины (1 байт, будет расширен при необходимости)
    //
    // Возвращает offset, по которому позже нужно вставить реальную длину.
    std::size_t beginSequence(uint8_t tag);

    // Завершить SEQUENCE — вычислить фактическую длину и
    // записать её на место placeholder.
    void endSequence(std::size_t anchorOffset);

    // Получить текущий размер буфера
    std::size_t size() const { return m_out.size(); }

private:
    std::vector<uint8_t>& m_out;
};

#endif // SNMP_BER_WRITER_H
