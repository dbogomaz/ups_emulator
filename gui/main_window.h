/**
 * @file main_window.h
 * @brief Главное окно графического интерфейса эмулятора UPS.
 *
 * Содержит класс MainWindow, реализующий основной
 * пользовательский интерфейс управления эмулятором UPS.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QActionGroup>
#include <QComboBox>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSpinBox>

#include "ups_emulator.h"

/// @ingroup gui

/**
 * @class MainWindow
 * @brief Главное окно GUI эмулятора UPS.
 *
 * Класс MainWindow отвечает за:
 * - отображение текущего состояния UPS;
 * - выбор модели устройства;
 * - управление параметрами эмулятора;
 * - запуск и остановку SNMP-агента через UpsEmulator.
 *
 * Класс не содержит доменной логики и использует
 * UpsEmulator как единственный источник данных и действий.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Создаёт главное окно GUI эмулятора UPS.
     *
     * @param parent Родительский QWidget (по умолчанию nullptr).
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Деструктор главного окна.
     */
    ~MainWindow() override = default;

private:
    UpsEmulator m_emulator;  ///< Экземпляр эмулятора UPS.

    // InputStatus widgets
    QSpinBox* m_inputVoltage_SpinBox{ nullptr };
    QSpinBox* m_inputFreq_SpinBox{ nullptr };
    // BatteryStatus widgets
    QSpinBox* m_chargeRemaining_SpinBox{ nullptr };
    QSpinBox* m_batteryTemp_SpinBox{ nullptr };
    QComboBox* m_batteryStatus_ComboBox{ nullptr };
    // OutputStatus widgets
    QSpinBox* m_outputVoltage_SpinBox{ nullptr };
    QComboBox* m_outputStatus_ComboBox{ nullptr };
    // Кнопки управления
    QPushButton* m_run_PushButton{ nullptr };
    QPushButton* m_stop_PushButton{ nullptr };
    QPushButton* m_exit_PushButton{ nullptr };
    // Меню
    QMenu* m_fileMenu{ nullptr };
    QMenu* m_modelMenu{ nullptr };
    QAction* m_exitAction{ nullptr };
    QActionGroup* m_modelActionGroup{ nullptr };

    /// @brief Обновляет элементы управления UI в соответствии с состоянием эмулятора.
    void updateRunStateUi();
    /// @brief Реализует выбор модели устройства.
    void modelSelected(QAction* action);
    /// @brief Обрабатывает нажатие кнопки "Run".
    void runPushButtonClicked();
    /// @brief Обрабатывает нажатие кнопки "Stop".
    void stopPushButtonClicked();
    /// @brief Применяет значения из GUI к эмулятору.
    void applyGuiValuesToEmulator();
};

#endif  // MAIN_WINDOW_H
