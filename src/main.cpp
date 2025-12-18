#include <algorithm>
#include <iostream>

#include "ups_emulator.h"

int main(int argc, char* argv[]) {
    const std::string configPath = "config/ups_models.ini";

    // Создаем эмулятор
    UpsEmulator emulator(configPath);

    if (!emulator.ok()) {
        std::cerr << "Failed to initialize emulator: " << emulator.lastError() << "\n";
        return 1;
    }

    // Получаем список моделей
    const auto& models = emulator.availableModels();
    if (models.empty()) {
        std::cerr << "No UPS models found.\n";
        return 1;
    }

    // Выбираем модель из аргументов командной строки или первую по умолчанию
    std::string cliModel;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-m" || arg == "--model") && i + 1 < argc) {
            cliModel = argv[i + 1];
            break;
        }
    }
    std::string modelName;
    if (!cliModel.empty() && std::find(models.begin(), models.end(), cliModel) != models.end()) {
        modelName = cliModel;
    } else {
        modelName = models.front();
    }
    if (!emulator.selectModel(modelName)) {
        std::cerr << "Failed to select model: " << emulator.lastError() << "\n";
        return 1;
    }

    // Основной цикл
    emulator.start();

    return 0;
}
