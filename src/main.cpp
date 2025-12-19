#include <algorithm>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include "ups_emulator.h"

static std::atomic<bool> g_stop{false};

void handleSignal(int) {
    g_stop.store(true);
}

int main(int argc, char* argv[]) {
    const std::string configPath = "config/ups_models.ini";

    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    UpsEmulator emulator(configPath);

    if (!emulator.ok()) {
        std::cerr << "Failed to initialize emulator: " << emulator.lastError() << "\n";
        return 1;
    }

    const auto& models = emulator.availableModels();
    if (models.empty()) {
        std::cerr << "No UPS models found.\n";
        return 1;
    }

    std::string cliModel;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-m" || arg == "--model") && i + 1 < argc) {
            cliModel = argv[i + 1];
            break;
        }
    }

    std::string modelName;
    if (!cliModel.empty() &&
        std::find(models.begin(), models.end(), cliModel) != models.end()) {
        modelName = cliModel;
    } else {
        modelName = models.front();
    }

    if (!emulator.selectModel(modelName)) {
        std::cerr << "Failed to select model: " << emulator.lastError() << "\n";
        return 1;
    }

    if (!emulator.start()) {
        std::cerr << "Failed to start emulator: " << emulator.lastError() << "\n";
        return 1;
    }

    // ===== CLI lifecycle =====
    while (!g_stop.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    emulator.stop();
    return 0;
}
