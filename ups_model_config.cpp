#include "ups_model_config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <map>

// -------------------
// trim helper
// -------------------
static inline std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// -------------------
// Получение каталога бинарника
// -------------------
std::string UpsModelConfig::getBinaryDir() const
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (len == -1)
        return ".";

    buf[len] = '\0';
    std::string path(buf);
    return path.substr(0, path.find_last_of('/'));
}

// -------------------
// Преобразование пути
// path может быть:
//   "/abs/path/file.ini" → абсолютный
//   "config/ups_models.ini" → относительный, тогда ищем около бинарника
// -------------------
std::string UpsModelConfig::resolvePath(const std::string& path) const
{
    if (!path.empty() && path[0] == '/')
        return path; // абсолютный путь

    std::string binDir = getBinaryDir();
    return binDir + "/" + path;
}

// -------------------
// Основная загрузка INI
// -------------------
bool UpsModelConfig::load(const std::string& path, const std::string& section)
{
    lastError.clear();
    modelName.clear();
    bypassValues.clear();
    oids = UpsOids{}; // сброс OID полей

    std::string fullPath = resolvePath(path);
    std::cout << ">>> Loading config from file: " << fullPath << "\n";

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        lastError = "Cannot open file: " + fullPath;
        return false;
    }

    std::string line;
    bool inSection = false;
    bool loadedAnything = false;

    // карта привязок "ключ - поле структуры"
    const std::map<std::string, std::string UpsOids::*> fieldMap = {
        {"modelNameOID",        &UpsOids::modelNameOID},
        {"inputVoltageOID",     &UpsOids::inputVoltageOID},
        {"inputFreqOID",        &UpsOids::inputFreqOID},
        {"outputVoltageOID",    &UpsOids::outputVoltageOID},
        {"batteryStatusOID",    &UpsOids::batteryStatusOID},
        {"chargeRemainingOID",  &UpsOids::chargeRemainingOID},
        {"batteryTempOID",      &UpsOids::batteryTempOID},
        {"bypassStatusOID",     &UpsOids::bypassStatusOID}
    };

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        // секция
        if (line.front() == '[' && line.back() == ']') {
            std::string sec = line.substr(1, line.size() - 2);
            inSection = (sec == section);
            continue;
        }

        if (!inSection)
            continue;

        // key=value
        size_t eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));

        // 1) modelName — отдельная сущность
        if (key == "modelName") {
            modelName = value;
            loadedAnything = true;
            continue;
        }

        // 2) поля OID
        auto it = fieldMap.find(key);
        if (it != fieldMap.end()) {
            oids.*(it->second) = value;
            loadedAnything = true;
            continue;
        }

        // 3) список значений байпаса
        if (key == "bypassStatusAllowed") {
            std::stringstream ss(value);
            std::string part;
            while (std::getline(ss, part, ',')) {
                part = trim(part);
                if (!part.empty()) {
                    try {
                        bypassValues.push_back(std::stoi(part));
                    } catch (...) {
                        lastError = "Invalid integer in bypassStatusAllowed: '" + part + "'";
                        return false;
                    }
                }
            }
            loadedAnything = true;
            continue;
        }

        // 4) неизвестный ключ
        std::cout << "WARNING: Unknown key in section [" << section << "]: " << key << "\n";
    }

    if (!loadedAnything) {
        lastError = "Section [" + section + "] was not found or empty in: " + fullPath;
        return false;
    }

    // обязательная проверка всех полей
    if (!validate(section))
        return false;

    return true;
}

// -------------------
// Проверка обязательных полей
// -------------------
bool UpsModelConfig::validate(const std::string& section)
{
    if (modelName.empty()) {
        lastError = "Missing required field \"modelName\" in section [" + section + "]";
        return false;
    }

    auto check = [&](const std::string& value, const std::string& name) {
        if (value.empty()) {
            lastError = "Missing required OID field \"" + name + "\" in section [" + section + "]";
            return false;
        }
        return true;
    };

    if (!check(oids.modelNameOID,       "modelNameOID")) return false;
    if (!check(oids.inputVoltageOID,    "inputVoltageOID")) return false;
    if (!check(oids.inputFreqOID,       "inputFreqOID")) return false;
    if (!check(oids.outputVoltageOID,   "outputVoltageOID")) return false;
    if (!check(oids.batteryStatusOID,   "batteryStatusOID")) return false;
    if (!check(oids.chargeRemainingOID, "chargeRemainingOID")) return false;
    if (!check(oids.batteryTempOID,     "batteryTempOID")) return false;
    if (!check(oids.bypassStatusOID,    "bypassStatusOID")) return false;

    if (bypassValues.empty()) {
        lastError = "Missing required list \"bypassStatusAllowed\" in section [" + section + "]";
        return false;
    }

    return true;
}
