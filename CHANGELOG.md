# Changelog
All notable changes to this project will be documented in this file.  
The format follows the principles of Keep a Changelog.

---

## [1.0.1] — 2025-12-11
### Changed
- Проведён рефакторинг архитектуры: создан отдельный класс `UpsEmulator`, который инкапсулирует всю логику работы эмулятора.  
  Функция `main()` теперь выполняет только роль входной точки и не содержит бизнес-логики.

---

## [1.0.0] — 2025-12-10
### Added
- Первая рабочая версия эмулятора ИБП.
- Реализована обработка SNMP-запросов типа GET для протоколов SNMP v1 и SNMP v2c.
- Добавлена поддержка модели APC.  
  (Файл конфигурации содержит секции для APC и INELT, однако в данной версии используется только APC.)
- Реализованы ответы для следующих OID:
  - `modelName`
  - `inputVoltage`
  - `inputFreq`
  - `outputVoltage`
  - `batteryStatus`
  - `chargeRemaining`
  - `batteryTemp`
  - `outputStatus`
