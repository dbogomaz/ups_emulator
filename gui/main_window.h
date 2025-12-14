#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QComboBox>
#include <QMainWindow>
#include <QSpinBox>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    // InputStatus
    QSpinBox* m_inputVoltage_SpinBox;
    QSpinBox* m_inputFreq_SpinBox;
    // BatteryStatus
    QSpinBox* m_chargeRemaining_SpinBox;
    QSpinBox* m_batteryTemp_SpinBox;
    QComboBox* m_batteryStatus_ComboBox;
    // OutputStatus
    QSpinBox* m_outputVoltage_SpinBox;
    QComboBox* m_outputStatus_ComboBox;

    QPushButton* m_run_PushButton;    
    QPushButton* m_stop_PushButton;    
    QPushButton* m_exit_PushButton;
};

#endif  // MAIN_WINDOW_H
