#include "snmp_ber_writer.h"

BerWriter::BerWriter(std::vector<uint8_t>& out) : m_out(out) {}

void BerWriter::putTag(uint8_t tag) {
    // TODO: реализовать
    m_out.push_back(tag);
}

void BerWriter::putLength(std::size_t len) {
    // TODO: реализовать short/long form
    (void)len;
}

void BerWriter::putByte(uint8_t b) { m_out.push_back(b); }

void BerWriter::putBytes(const uint8_t* data, std::size_t len) {
    // TODO: реализовать
    (void)data;
    (void)len;
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
