/**
 * @file ups_data_store.h
 * @brief Хранилище текущего состояния UPS.
 *
 * Содержит класс, отвечающий за хранение, получение и изменение
 * параметров источника бесперебойного питания во время работы эмулятора.
 */
#ifndef UPS_DATA_STORE_H
#define UPS_DATA_STORE_H

#include <mutex>
#include <unordered_map>

#include "ups_model_config.h"
#include "ups_parameter.h"

/// @ingroup ups

/**
 * @class UpsDataStore
 * @brief Потокобезопасное хранилище параметров UPS.
 *
 * Класс отвечает за:
 * - хранение текущих значений параметров UPS;
 * - инициализацию параметров на основе конфигурации модели;
 * - валидацию значений перечислимых параметров;
 * - потокобезопасный доступ к данным.
 *
 * Используется несколькими подсистемами проекта, включая
 * SNMP-агент, эмулятор и графический интерфейс.
 */
class UpsDataStore {
public:
    /**
     * @brief Инициализирует хранилище по конфигурации модели UPS.
     *
     * Создаёт набор параметров и заполняет их начальными значениями
     * на основе конфигурации модели.
     *
     * @param cfg Конфигурация модели UPS.
     * @return true в случае успешной инициализации.
     */
    bool init(const UpsModelConfig& cfg);

    /**
     * @brief Получает параметр UPS по его SNMP OID.
     *
     * Метод потокобезопасен.
     *
     * @param oid SNMP OID параметра.
     * @param out Выходная структура с параметром.
     * @return true, если параметр найден.
     */
    bool get(const Oid& oid, UpsParameter& out) const;

    /**
     * @brief Устанавливает новое значение параметра UPS.
     *
     * Выполняет проверку допустимости значения для перечислимых
     * параметров. Метод потокобезопасен.
     *
     * @param oid SNMP OID параметра.
     * @param value Новое значение параметра (в строковом виде).
     * @param err Указатель для записи сообщения об ошибке (опционально).
     * @return true, если значение успешно установлено.
     */
    bool set(const Oid& oid, const UpsParameterValue& value, ErrorMessage* err = nullptr);

private:
    /// Хранилище параметров UPS (ключ — SNMP OID).
    std::unordered_map<Oid, UpsParameter> m_parameters{};

    /// Ограничения для перечислимых параметров (OID → набор допустимых значений).
    std::unordered_map<Oid, const FieldValueSet*> m_valueSets{};

    /// Мьютекс для обеспечения потокобезопасного доступа.
    mutable std::mutex m_mutex;
};

#endif  // UPS_DATA_STORE_H
