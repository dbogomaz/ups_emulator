/**
 * @file main.cpp
 * @brief Точка входа GUI-приложения эмулятора UPS.
 *
 * Инициализирует QApplication, создаёт и отображает
 * главное окно графического интерфейса (MainWindow).
 */
#include <QApplication>

#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
