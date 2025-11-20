#include "ups_model_config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <limits.h>

#include <iostream>

static inline std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool UpsModelConfig::load(const std::string& path, const std::string& section)
{
    std::string binDir = getBinaryDir();
    std::string fullPath = binDir + "/" + path;
    std::cout << ">>> Loading config from file: " << fullPath << std::endl;
    std::ifstream file(fullPath);
    if (!file.is_open()) return false;

    std::string line;
    bool inSection = false;

    // Карта ключей - поля UpsOids
    const std::map<std::string, std::string UpsOids::*> fieldMap = {
        {"modelNameOID",        &UpsOids::modelNameOID},
        {"inputVoltageOID",     &UpsOids::inputVoltageOID},
        {"inputFreqOID",        &UpsOids::inputFreqOID},
        {"outputVoltageOID",    &UpsOids::outputVoltageOID},
        {"batteryStatusOID",    &UpsOids::batteryStatusOID},
        {"chargeRemainingOID",  &UpsOids::chargeRemainingOID},
        {"batteryTempOID",      &UpsOids::batteryTempOID},
        {"bypassStatusOID",     &UpsOids::bypassStatusOID},
    };

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        // секция
        if (line.front() == '[' && line.back() == ']') {
            std::string sec = line.substr(1, line.size() - 2);
            inSection = (sec == section);
            continue;
        }

        if (!inSection) continue;

        // парсим ключ=значение
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        // Простые поля
        if (key == "modelName") {
            modelName = value;
            continue;
        }

        // Поля UpsOids
        auto it = fieldMap.find(key);
        if (it != fieldMap.end()) {
            oids.*(it->second) = value;
            continue;
        }

        // Поля с allowed
        if (key == "bypassStatusAllowed") {
            std::stringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                item = trim(item);
                if (!item.empty()) {
                    bypassValues.push_back(std::stoi(item));
                }
            }
            continue;
        }
    }

    return true;
}


std::string UpsModelConfig::getBinaryDir() const {
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len == -1) return ".";
    buf[len] = '\0';
    std::string path(buf);
    return path.substr(0, path.find_last_of('/'));
}


