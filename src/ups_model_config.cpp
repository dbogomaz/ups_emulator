#include "ups_model_config.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "utils/fs_utils.h"
#include "utils/string_utils.h"

bool UpsModelConfig::load(const std::string& path, const std::string& section) {
    m_lastError.clear();
    m_modelName.clear();
    m_bypassValues.clear();
    m_oids = UpsOids{};  // сброс OID полей

    std::string fullPath = utils::resolvePath(path);

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file: " + fullPath;
        return false;
    }

    std::string line;
    bool inSection = false;       // флаг, что мы внутри нужной секции
    bool loadedAnything = false;  // флаг, что мы загрузили хоть что-то

    // LCOV_EXCL_START
    // карта привязок "ключ - поле структуры"
    const std::map<std::string, std::string UpsOids::*> fieldMap = {
        {"modelNameOID", &UpsOids::modelNameOID},
        {"inputVoltageOID", &UpsOids::inputVoltageOID},
        {"inputFreqOID", &UpsOids::inputFreqOID},
        {"outputVoltageOID", &UpsOids::outputVoltageOID},
        {"batteryStatusOID", &UpsOids::batteryStatusOID},
        {"chargeRemainingOID", &UpsOids::chargeRemainingOID},
        {"batteryTempOID", &UpsOids::batteryTempOID},
        {"bypassStatusOID", &UpsOids::bypassStatusOID}};
    // LCOV_EXCL_STOP

    while (std::getline(file, line)) {
        line = utils::trim(line);
        if (line.empty() || line[0] == '#') continue;

        // секция
        if (line.front() == '[' && line.back() == ']') {
            std::string sec = line.substr(1, line.size() - 2);
            inSection = (sec == section);
            continue;
        }

        if (!inSection) continue;

        // key=value
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = utils::trim(line.substr(0, eq));
        std::string value = utils::trim(line.substr(eq + 1));

        // 1) modelName — отдельная сущность
        if (key == "modelName") {
            m_modelName = value;
            loadedAnything = true;
            continue;
        }

        // 2) поля OID
        auto it = fieldMap.find(key);
        if (it != fieldMap.end()) {
            m_oids.*(it->second) = value;
            loadedAnything = true;
            continue;
        }

        // 3) список значений байпаса
        if (key == "bypassStatusAllowed") {
            std::stringstream ss(value);
            std::string part;
            while (std::getline(ss, part, ',')) {
                part = utils::trim(part);
                if (!part.empty()) {
                    try {
                        m_bypassValues.push_back(std::stoi(part));
                    } catch (...) {
                        m_lastError = "Invalid integer in bypassStatusAllowed: '" + part + "'";
                        return false;
                    }
                }
            }
            loadedAnything = true;
            continue;
        }
    }

    if (!loadedAnything) {
        m_lastError = "Section [" + section + "] was not found or empty in: " + fullPath;
        return false;
    }

    // обязательная проверка всех полей
    if (!validate(section)) return false;

    return true;
}

bool UpsModelConfig::validate(const std::string& section) {
    if (m_modelName.empty()) {
        m_lastError = "Missing required field \"modelName\" in section [" + section + "]";
        return false;
    }

    auto check = [&](const std::string& value, const std::string& name) {
        if (value.empty()) {
            m_lastError =
                "Missing required OID field \"" + name + "\" in section [" + section + "]";
            return false;
        }
        return true;
    };

    if (!check(m_oids.modelNameOID, "modelNameOID")) return false;
    if (!check(m_oids.inputVoltageOID, "inputVoltageOID")) return false;
    if (!check(m_oids.inputFreqOID, "inputFreqOID")) return false;
    if (!check(m_oids.outputVoltageOID, "outputVoltageOID")) return false;
    if (!check(m_oids.batteryStatusOID, "batteryStatusOID")) return false;
    if (!check(m_oids.chargeRemainingOID, "chargeRemainingOID")) return false;
    if (!check(m_oids.batteryTempOID, "batteryTempOID")) return false;
    if (!check(m_oids.bypassStatusOID, "bypassStatusOID")) return false;

    if (m_bypassValues.empty()) {
        m_lastError = "Missing required list \"bypassStatusAllowed\" in section [" + section + "]";
        return false;
    }

    return true;
}

const std::string& UpsModelConfig::modelName() const { return m_modelName; }

const UpsOids& UpsModelConfig::oids() const { return m_oids; }

const std::vector<int>& UpsModelConfig::bypassValues() const { return m_bypassValues; }

const std::string& UpsModelConfig::lastError() const { return m_lastError; }
