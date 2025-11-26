#include "ups_model_config.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "utils/fs_utils.h"
#include "utils/string_utils.h"

bool UpsModelConfig::load(const std::string& path, const std::string& section) {
    m_modelName.clear();
    m_oids = UpsOids{};    // сброс OID полей
    m_definedFields = FieldValueSets{};  // сброс набора сложных полей
    m_lastError.clear();

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
        {"outputStatusOID", &UpsOids::outputStatusOID}};
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

        if (key == "batteryStatusValues") {
            std::string full = utils::readMultilineBracedBlock(file, value);

            if (!parseFieldValueSet(full, m_definedFields.batteryStatusSet)) {
                m_lastError = "Invalid format in batteryStatusValues: " + full;
                return false;
            }
            loadedAnything = true;
            continue;
        }

        if (key == "outputStatusValues") {
            std::string full = utils::readMultilineBracedBlock(file, value);
            if (!parseFieldValueSet(full, m_definedFields.outputStatusSet)) {
                m_lastError = "Invalid format in outputStatusValues: " + full;
                return false;
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
    if (!check(m_oids.outputStatusOID, "outputStatusOID")) return false;

    if (m_definedFields.batteryStatusSet.nameToValue.empty()) {
        m_lastError = "Field \"batteryStatusValues\" is missing or empty";
        return false;
    }

    if (m_definedFields.outputStatusSet.nameToValue.empty()) {
        m_lastError = "Field \"outputStatusValues\" is missing or empty";
        return false;
    }

    return true;
}

bool UpsModelConfig::parseFieldValueSet(const std::string& raw, FieldValueSet& out) {
    out.nameToValue.clear();
    out.valueToName.clear();

    std::string s = utils::trim(raw);

    if (s.size() < 2 || s.front() != '{' || s.back() != '}') return false;

    s = s.substr(1, s.size() - 2);  // remove {}

    std::stringstream ss{s};
    std::string pair;

    while (std::getline(ss, pair, ',')) {
        auto pos = pair.find(':');
        if (pos == std::string::npos) return false;

        std::string name = utils::trim(pair.substr(0, pos));
        std::string val = utils::trim(pair.substr(pos + 1));

        // remove quotes
        if (name.size() >= 2 && name.front() == '"' && name.back() == '"')
            name = name.substr(1, name.size() - 2);

        int number = 0;
        try {
            number = std::stoi(val);
        } catch (const std::exception&) {
            m_lastError = "Invalid integer value in enum: '" + val + "'";
            return false;
        }

        out.nameToValue[name] = number;
        out.valueToName[number] = name;
    }
    return true;
}

const std::string& UpsModelConfig::modelName() const { return m_modelName; }

const UpsOids& UpsModelConfig::oids() const { return m_oids; }

const FieldValueSets& UpsModelConfig::definedFields() const { return m_definedFields; }

const std::string& UpsModelConfig::lastError() const { return m_lastError; }
