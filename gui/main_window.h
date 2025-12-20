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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    UpsEmulator m_emulator;

    // InputStatus
    QSpinBox* m_inputVoltage_SpinBox{ nullptr };
    QSpinBox* m_inputFreq_SpinBox{ nullptr };
    // BatteryStatus
    QSpinBox* m_chargeRemaining_SpinBox{ nullptr };
    QSpinBox* m_batteryTemp_SpinBox{ nullptr };
    QComboBox* m_batteryStatus_ComboBox{ nullptr };
    // OutputStatus
    QSpinBox* m_outputVoltage_SpinBox{ nullptr };
    QComboBox* m_outputStatus_ComboBox{ nullptr };

    QPushButton* m_run_PushButton{ nullptr };
    QPushButton* m_stop_PushButton{ nullptr };
    QPushButton* m_exit_PushButton{ nullptr };

    QMenu* m_fileMenu{ nullptr };
    QMenu* m_modelMenu{ nullptr };

    QAction* m_exitAction{ nullptr };
    QActionGroup* m_modelActionGroup{ nullptr };

    void updateRunStateUi();

    void modelSelected(QAction* action);

    void runPushButtonClicked();
    void stopPushButtonClicked();

    void applyGuiValuesToEmulator();
};

#endif  // MAIN_WINDOW_H
