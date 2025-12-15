#include <iostream>

#include "ups_emulator.h"

int main() {
    const std::string configPath = "config/ups_models.ini";

    // Создаем эмулятор
    UpsEmulator emulator(configPath);

    if (!emulator.ok()) {
        std::cerr << "Failed to initialize emulator: "
                  << emulator.lastError() << "\n";
        return 1;
    }

    // Получаем список моделей
    const auto& models = emulator.availableModels();
    if (models.empty()) {
        std::cerr << "No UPS models found.\n";
        return 1;
    }

    // Выбираем первую модель из списка
    const auto& modelName = models[0];
    std::cout << "Using model: " << modelName << "\n";

    if (!emulator.selectModel(modelName)) {
        std::cerr << "Failed to select model: "
                  << emulator.lastError() << "\n";
        return 1;
    }

    // Основной цикл
    emulator.start();

    return 0;
}
