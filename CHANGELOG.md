# Changelog
All notable changes to this project will be documented in this file.  
The format follows the principles of Keep a Changelog.

---

## [1.0.2] — 2025-12-12
### Fixed
- Исправлена логика запуска и остановки SNMP-агента.
- Устранена блокировка при `recvfrom()` за счёт перехода на `poll()`.

### Changed
- Жизненный цикл `SnmpAgent` приведён к явной модели состояний (`bind / run / stop / isRunning`).
- Добавлена защита от повторного вызова `bind()`.

### Added
- Модульные тесты, покрывающие жизненный цикл `SnmpAgent`.

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
