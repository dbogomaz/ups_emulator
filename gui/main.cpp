/**
 * @file main.cpp
 * @brief Точка входа GUI-приложения эмулятора UPS.
 *
 * Инициализирует QApplication, создаёт и отображает
 * главное окно графического интерфейса (MainWindow).
 */
#include <QApplication>

#include "main_window.h"

/**
 * @brief Точка входа GUI-приложения UPS Emulator.
 *
 * @param argc Количество аргументов командной строки
 * @param argv Аргументы командной строки
 * @return Код завершения приложения
 */
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

