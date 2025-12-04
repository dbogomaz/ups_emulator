#include "snmp_ber_writer.h"

BerWriter::BerWriter(std::vector<uint8_t>& out) : m_out(out) {}

void BerWriter::putTag(uint8_t tag) { putByte(tag); }

void BerWriter::putLength(std::size_t contentLength) {
    // Short form
    if (contentLength < 128) {
        putByte(static_cast<uint8_t>(contentLength));
        return;
    }

    // Long form
    // Определяем минимальное количество байт, чтобы представить len
    if (contentLength <= 0xFF) {
        // 1 byte length
        putByte(0x81);
        putByte(static_cast<uint8_t>(contentLength));
    } else if (contentLength <= 0xFFFF) {
        // 2 byte length
        putByte(0x82);
        putByte(static_cast<uint8_t>((contentLength >> 8) & 0xFF));
        putByte(static_cast<uint8_t>(contentLength & 0xFF));
    } else {
        // Теоретически SNMP не использует такие длины, но реализуем 3 bytes
        putByte(0x83);
        putByte(static_cast<uint8_t>((contentLength >> 16) & 0xFF));
        putByte(static_cast<uint8_t>((contentLength >> 8) & 0xFF));
        putByte(static_cast<uint8_t>(contentLength & 0xFF));
    }
}

void BerWriter::putByte(uint8_t b) { m_out.push_back(b); }

void BerWriter::putBytes(const uint8_t* data, std::size_t len) {
    if (!data || 
        len == 0) 
        return;
    m_out.insert(m_out.end(), data, data + len);
}

std::size_t BerWriter::beginSequence(uint8_t tag) {
    // TODO:
    // 1. putTag(tag)
    // 2. put placeholder length (e.g. 0x00)
    // 3. return offset of length placeholder
    (void)tag;
    return 0;
}

void BerWriter::endSequence(std::size_t anchorOffset) {
    // TODO:
    // 1. compute body length = current_size - (anchorOffset + placeholder_size)
    // 2. overwrite placeholder with correct length encoding
    // 3. если длина требует long-form — сдвинуть хвостовые байты
    (void)anchorOffset;
}
