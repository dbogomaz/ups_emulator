/**
 * @file ups_model_config.h
 * @brief Конфигурация модели UPS, загружаемая из INI-файла.
 *
 * Содержит класс, отвечающий за загрузку, хранение и валидацию
 * конфигурации модели источника бесперебойного питания.
 */
#ifndef UPS_MODEL_CONFIG_H
#define UPS_MODEL_CONFIG_H

#include "ups_oids.h"
#include "ups_value_sets.h"

/// @ingroup ups

/**
 * @class UpsModelConfig
 * @brief Представляет конфигурацию модели UPS.
 *
 * Класс отвечает за:
 * - загрузку конфигурации модели UPS из INI-файла;
 * - хранение имени модели и соответствующих SNMP OID;
 * - хранение описаний перечислимых параметров;
 * - валидацию структуры и содержимого конфигурации.
 *
 * Используется на этапе инициализации эмулятора и хранилища данных.
 */
class UpsModelConfig {
public:
    /**
     * @brief Загружает конфигурацию модели UPS из INI-файла.
     *
     * @param path Путь к INI-файлу с описанием моделей.
     * @param section Имя секции, соответствующей модели UPS.
     * @return true, если конфигурация успешно загружена и валидна.
     */
    bool load(const std::string& path, const IniSectionName& section);

    /**
     * @brief Возвращает имя модели UPS.
     * @return Имя модели UPS.
     */
    const ModelName& modelName() const;

    /**
     * @brief Возвращает набор SNMP OID модели UPS.
     * @return Набор SNMP OID модели UPS.
     */
    const UpsOids& oids() const;

    /**
     * @brief Возвращает описания перечислимых параметров модели UPS.
     * @return Описания перечислимых параметров модели UPS.
     */
    const FieldValueSets& definedFields() const;

    /**
     * @brief Возвращает описание последней ошибки.
     * @return Текст последней ошибки.
     */
    const ErrorMessage& lastError() const;

private:
    /// Имя модели UPS.
    ModelName m_modelName{};

    /// Набор SNMP OID модели UPS.
    UpsOids m_oids{};

    /// Описания перечислимых параметров модели UPS.
    FieldValueSets m_definedFields{};

    /// Текст последней ошибки.
    ErrorMessage m_lastError{};

    /**
     * @brief Выполняет валидацию загруженной конфигурации.
     */
    bool validate(const IniSectionName& section);

    /**
     * @brief Разбирает описание набора значений параметра.
     *
     * @param raw Сырой текст из конфигурации.
     * @param out Структура для заполнения.
     * @return true в случае успешного разбора.
     */
    bool parseFieldValueSet(const std::string& raw, FieldValueSet& out);
};

#endif  // UPS_MODEL_CONFIG_H
