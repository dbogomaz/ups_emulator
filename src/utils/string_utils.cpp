#include "string_utils.h"

#include <fstream>

namespace utils {

std::string trim(const std::string& s) {
    const char* WS = " \t\r\n";

    size_t start = s.find_first_not_of(WS);
    if (start == std::string::npos) return "";

    size_t end = s.find_last_not_of(WS);
    return s.substr(start, end - start + 1);
}

std::string readMultilineBracedBlock(std::ifstream& file, const std::string& firstLine) {
    std::string result = firstLine;

    // Если это однострочный enum — возвращаем как есть
    if (firstLine.find('}') != std::string::npos) return result;

    std::string line;
    while (std::getline(file, line)) {
        line = utils::trim(line);

        if (line.empty() || line[0] == '#') continue;

        result += " " + line;

        if (line.find('}') != std::string::npos) break;
    }
    return result;
}

}  // namespace utils
