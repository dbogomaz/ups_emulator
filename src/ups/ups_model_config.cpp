#include "ups_model_config.h"

#include <fstream>
#include <sstream>
#include <algorithm>

#include "utils/fs_utils.h"
#include "utils/string_utils.h"

// =============================================================
//  Таблица обычных OID-полей
// =============================================================
struct OidFieldInfo {
    const char* key;
    std::string UpsOids::* member;
};

static const OidFieldInfo OID_FIELDS[] = {
    {"modelNameOID", &UpsOids::modelNameOID},
    {"inputVoltageOID", &UpsOids::inputVoltageOID},
    {"inputFreqOID", &UpsOids::inputFreqOID},
    {"outputVoltageOID", &UpsOids::outputVoltageOID},
    {"batteryStatusOID", &UpsOids::batteryStatusOID},
    {"chargeRemainingOID", &UpsOids::chargeRemainingOID},
    {"batteryTempOID", &UpsOids::batteryTempOID},
    {"outputStatusOID", &UpsOids::outputStatusOID},
};

// =============================================================
//  Таблица сложных полей  { "Name": 1, "Other": 2 }
// =============================================================
struct ValueSetFieldInfo {
    const char* key;
    FieldValueSet FieldValueSets::* member;
};

static const ValueSetFieldInfo VALUESET_FIELDS[] = {
    {"batteryStatusValues", &FieldValueSets::batteryStatusSet},
    {"outputStatusValues", &FieldValueSets::outputStatusSet},
};

// =============================================================
//  Вспомогательная функция: найти сложное поле по ключу
// =============================================================
static const ValueSetFieldInfo* findValueSetField(const std::string& key) {
    for (const auto& f : VALUESET_FIELDS) {
        if (key == f.key) return &f;
    }
    return nullptr;
}

// =============================================================
//  Основная загрузка INI-секции
// =============================================================
bool UpsModelConfig::load(const std::string& path, const std::string& section) {
    m_modelName.clear();
    m_oids = UpsOids{};
    m_definedFields = FieldValueSets{};
    m_lastError.clear();

    std::string fullPath = utils::resolvePath(path);

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        m_lastError = "Cannot open file: " + fullPath;
        return false;
    }

    std::string line;
    bool inSection = false;
    bool loadedAnything = false;

    while (std::getline(file, line)) {
        line = utils::trim(line);

        // ---- пропустить пустые строки и комментарии ----
        if (line.empty() || 
            line[0] == '#') continue;

        // ---- определение секции ----
        if (line.front() == '[' && 
            line.back() == ']') {
            std::string sec = line.substr(1, line.size() - 2);
            inSection = (sec == section);
            continue;
        }
        if (!inSection) continue;

        // ---- должен быть знак '=' ----
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        // если дошли до сюда, то есть key=value
        std::string key = utils::trim(line.substr(0, eq));
        std::string value = utils::trim(line.substr(eq + 1));

        // ---- modelName ----
        if (key == "modelName") {
            m_modelName = value;
            loadedAnything = true;
            continue;
        }

        // ---- обычные OID-поля ----
        bool isOidField = false;
        for (const auto& f : OID_FIELDS) {
            if (key == f.key) {
                m_oids.*(f.member) = value;
                loadedAnything = true;
                isOidField = true;
                break;
            }
        }
        if (isOidField) continue;

        // ---- сложные поля ----
        if (const auto* f = findValueSetField(key)) {
            std::string fullBlock = utils::readMultilineBracedBlock(file, value);
            if (!parseFieldValueSet(fullBlock, m_definedFields.*(f->member))) {
                // lastError уже установлен в parseFieldValueSet
                return false;
            }
            loadedAnything = true;
            continue;
        }

        continue;
    }

    // ---- секция не найдена или пуста ----
    if (!loadedAnything) {
        m_lastError = "Section [" + section + "] was not found or empty in: " + fullPath;
        return false;
    }

    return validate(section);
}

// =============================================================
//  Проверка полей
// =============================================================
bool UpsModelConfig::validate(const std::string& section) {
    // ---- modelName ----
    if (m_modelName.empty()) {
        m_lastError = "Missing required field \"modelName\" in section [" + section + "]";
        return false;
    }

    // ---- все OID поля ----
    for (const auto& f : OID_FIELDS) {
        const std::string& val = m_oids.*(f.member);
        if (val.empty()) {
            m_lastError = "Missing required OID field \"" + std::string(f.key) + "\" in section [" +
                          section + "]";
            return false;
        }
    }

    // ---- все сложные поля ----
    for (const auto& f : VALUESET_FIELDS) {
        const auto& set = m_definedFields.*(f.member);
        if (set.nameToValue.empty()) {
            m_lastError = "Field \"" + std::string(f.key) + "\" is missing or empty";
            return false;
        }
    }

    return true;
}

// =============================================================
//  Разбор структуры сложного поля { "Name": 1, "Other": 2 }
// =============================================================
bool UpsModelConfig::parseFieldValueSet(const std::string& raw, FieldValueSet& out) {
    printf("[DEBUG] <parseFieldValueSet> raw: %s\n", raw.c_str());
    out.nameToValue.clear();
    out.valueToName.clear();

    // ----  проверка парности скобок в raw ----
    int openCount = std::count(raw.begin(), raw.end(), '{');
    int closeCount = std::count(raw.begin(), raw.end(), '}');
    if (openCount != 1 || 
        closeCount != 1) {
        m_lastError = "Invalid value-set block: unbalanced braces";
        return false;
    }

    std::string s = utils::trim(raw);
    // ---- проверка наличия внешних фигурных скобок ----
    if (s.size() < 2 || 
        s.front() != '{' || 
        s.back() != '}') {
        m_lastError = "Invalid value-set format: " + raw;
        return false;
    }

    // удаление внешних фигурных скобок {}
    s = s.substr(1, s.size() - 2);

    std::stringstream ss{s};
    std::string pair;

    while (std::getline(ss, pair, ',')) {
        auto pos = pair.find(':');
        if (pos == std::string::npos) {
            m_lastError = "Invalid value-set entry: " + pair;
            return false;
        }

        // если внутри пары есть кавычки после значения — значит пропущена запятая
        if (pair.find('"', pos + 1) != std::string::npos) {
            m_lastError = "Invalid value-set entry (missing comma): " + pair;
            return false;
        }
        std::string name = utils::trim(pair.substr(0, pos));
        std::string val = utils::trim(pair.substr(pos + 1));

        // ---- remove quotes ----
        if (name.size() >= 2 && 
            name.front() == '"' && 
            name.back() == '"')
            name = name.substr(1, name.size() - 2);

        int number = 0;
        try {
            number = std::stoi(val);
        } catch (...) {
            m_lastError = "Invalid integer in value-set: '" + val + "'";
            return false;
        }

        out.nameToValue[name] = number;
        out.valueToName[number] = name;
    }

    return true;
}

// =============================================================
//  Getters
// =============================================================
const std::string& UpsModelConfig::modelName() const { return m_modelName; }
const UpsOids& UpsModelConfig::oids() const { return m_oids; }
const FieldValueSets& UpsModelConfig::definedFields() const { return m_definedFields; }
const std::string& UpsModelConfig::lastError() const { return m_lastError; }
