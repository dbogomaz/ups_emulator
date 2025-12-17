#include "main_window.h"

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_emulator("config/ups_models.ini") {
    setWindowTitle("UPS Emulator");
    resize(600, 200);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    m_inputVoltage_SpinBox = new QSpinBox(central);
    m_inputVoltage_SpinBox->setMinimum(0);
    m_inputVoltage_SpinBox->setMaximum(300);
    m_inputVoltage_SpinBox->setValue(230);

    m_inputFreq_SpinBox = new QSpinBox(central);
    m_inputFreq_SpinBox->setMinimum(0);
    m_inputFreq_SpinBox->setMaximum(999);
    m_inputFreq_SpinBox->setValue(500);

    m_chargeRemaining_SpinBox = new QSpinBox(central);
    m_chargeRemaining_SpinBox->setMinimum(0);
    m_chargeRemaining_SpinBox->setMaximum(100);
    m_chargeRemaining_SpinBox->setValue(80);

    m_batteryTemp_SpinBox = new QSpinBox(central);
    m_batteryTemp_SpinBox->setMinimum(0);
    m_batteryTemp_SpinBox->setMaximum(100);
    m_batteryTemp_SpinBox->setValue(30);

    m_batteryStatus_ComboBox = new QComboBox(central);

    m_outputVoltage_SpinBox = new QSpinBox(central);
    m_outputVoltage_SpinBox->setMinimum(0);
    m_outputVoltage_SpinBox->setMaximum(300);
    m_outputVoltage_SpinBox->setValue(230);

    m_run_PushButton = new QPushButton("Run", central);
    m_stop_PushButton = new QPushButton("Stop", central);
    m_exit_PushButton = new QPushButton("Exit", central);

    m_outputStatus_ComboBox = new QComboBox(central);

    QVBoxLayout* main_Layout = new QVBoxLayout(central);
    {
        // виджеты с параметрами
        QHBoxLayout* parameters_Layout = new QHBoxLayout;
        {
            QVBoxLayout* inputStatus_Layout = new QVBoxLayout;
            {
                QLabel* inputStatus_Label = new QLabel("InputStatus");
                QFormLayout* inputStatus_FormLayout = new QFormLayout;
                {
                    inputStatus_FormLayout->addRow("inputVoltage", m_inputVoltage_SpinBox);
                    inputStatus_FormLayout->addRow("inputFreq", m_inputFreq_SpinBox);
                }

                inputStatus_Layout->addWidget(inputStatus_Label);
                inputStatus_Layout->addLayout(inputStatus_FormLayout);
                inputStatus_Layout->addStretch();
            }

            QVBoxLayout* batteryStatus_Layout = new QVBoxLayout;
            {
                QLabel* batteryStatus_Label = new QLabel("BatteryStatus");
                QFormLayout* batteryStatus_FormLayout = new QFormLayout;
                {
                    batteryStatus_FormLayout->addRow("chargeRemaining", m_chargeRemaining_SpinBox);
                    batteryStatus_FormLayout->addRow("batteryTemp", m_batteryTemp_SpinBox);
                    batteryStatus_FormLayout->addRow("batteryStatus", m_batteryStatus_ComboBox);
                }

                batteryStatus_Layout->addWidget(batteryStatus_Label);
                batteryStatus_Layout->addLayout(batteryStatus_FormLayout);
                batteryStatus_Layout->addStretch();
            }

            QVBoxLayout* outputStatus_Layout = new QVBoxLayout;
            {
                QLabel* outputStatus_Label = new QLabel("OutputStatus");
                QFormLayout* outputStatus_FormLayout = new QFormLayout;
                {
                    outputStatus_FormLayout->addRow("outputVoltage", m_outputVoltage_SpinBox);
                    outputStatus_FormLayout->addRow("outputStatus", m_outputStatus_ComboBox);
                }

                outputStatus_Layout->addWidget(outputStatus_Label);
                outputStatus_Layout->addLayout(outputStatus_FormLayout);
                outputStatus_Layout->addStretch();
            }

            parameters_Layout->addLayout(inputStatus_Layout);
            parameters_Layout->addLayout(batteryStatus_Layout);
            parameters_Layout->addLayout(outputStatus_Layout);
        }

        // кнопки
        QHBoxLayout* buttons_Layout = new QHBoxLayout;
        {
            buttons_Layout->addStretch();
            buttons_Layout->addWidget(m_run_PushButton);
            buttons_Layout->addWidget(m_stop_PushButton);
            buttons_Layout->addWidget(m_exit_PushButton);
        }

        main_Layout->addLayout(parameters_Layout);
        main_Layout->addLayout(buttons_Layout);
    }

    // ===== MenuBar =====

    // --- File ---
    m_fileMenu = menuBar()->addMenu("File");
    m_exitAction = new QAction("Exit", this);
    m_fileMenu->addAction(m_exitAction);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    // --- Model ---
    m_modelMenu = menuBar()->addMenu("Model");
    m_modelActionGroup = new QActionGroup(this);
    m_modelActionGroup->setExclusive(true);
    const auto models = m_emulator.availableModels();
    for (const auto& name : models) {
        QAction* action = new QAction(QString::fromStdString(name), this);
        action->setCheckable(true);
        m_modelActionGroup->addAction(action);
        m_modelMenu->addAction(action);
    }
    connect(m_modelActionGroup, &QActionGroup::triggered, this, &MainWindow::modelSelected);
    if (!models.empty()) {
         m_modelActionGroup->actions().front()->trigger();
    }

    menuBar()->setNativeMenuBar(false);  // временно

    updateRunStateUi();

    connect(m_run_PushButton, &QPushButton::clicked, this, &MainWindow::runPushButtonClicked);
    connect(m_stop_PushButton, &QPushButton::clicked, this, &MainWindow::stopPushButtonClicked);
    connect(m_exit_PushButton, &QPushButton::clicked, this, &QWidget::close);
}

void MainWindow::updateRunStateUi() {
    const bool running = m_emulator.isRunning();
    m_run_PushButton->setEnabled(!running);
    m_stop_PushButton->setEnabled(running);
    m_modelMenu->setEnabled(!running);
}

void MainWindow::modelSelected(QAction* action) {
    if (m_emulator.isRunning()) {
        return;
    }
    m_emulator.selectModel(action->text().toStdString());

    const IniSectionName model = m_emulator.currentModel();
    const std::string windowsTitlePrefix = "UPS Emulator — ";
    if (!model.empty()) {
        setWindowTitle(QString::fromStdString(windowsTitlePrefix) + QString::fromStdString(model));
    } else {
        setWindowTitle(QString::fromStdString(windowsTitlePrefix) + QString("No model selected"));
    }

    // Заполняем ComboBox для BatteryStatus
    m_batteryStatus_ComboBox->clear();
    const auto& batteryStatuses = m_emulator.availableBatteryStatuses();
    for (const auto& status : batteryStatuses) {
        m_batteryStatus_ComboBox->addItem(QString::fromStdString(status));
    }

    // Заполняем ComboBox для OutputStatus
    m_outputStatus_ComboBox->clear();
    const auto& outputStatuses = m_emulator.availableOutputStatuses();
    for (const auto& status : outputStatuses) {
        m_outputStatus_ComboBox->addItem(QString::fromStdString(status));
    }
}

void MainWindow::runPushButtonClicked() {
    if (m_emulator.isRunning()) {
        return;
    }
    m_emulator.start();
    updateRunStateUi();
}

void MainWindow::stopPushButtonClicked() {
    m_emulator.stop();
    updateRunStateUi();
}
