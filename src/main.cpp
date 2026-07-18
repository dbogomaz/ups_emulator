#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <csignal>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

#include "ups_emulator.h"

static std::atomic<bool> g_stop{ false };
void handleSignal(int);

struct CliOptions {
    bool showHelp = false;
    bool listModels = false;
    std::string modelId;
    uint16_t port = 161;
    std::string error;
};

CliOptions parseArgs(int argc, char* argv[]);
void printHelp(const char* progName);

int main(int argc, char* argv[]) {
    const std::string configPath = "config/ups_models.ini";

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    CliOptions opts = parseArgs(argc, argv);

    if (!opts.error.empty()) {
        std::cerr << "Error: " << opts.error << "\n";
        std::cerr << "Use --help to see available options.\n";
        return 1;
    }

    if (opts.showHelp) {
        printHelp(argv[0]);
        return 0;
    }

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

    if (opts.listModels) {
        for (const auto& id : models) {
            std::cout << id << "\n";
        }
        return 0;
    }

    std::string modelName = opts.modelId.empty() ? models.front() : opts.modelId;

    if (std::find(models.begin(), models.end(), modelName) == models.end()) {
        std::cerr << "Unknown UPS model: " << modelName << "\n";
        return 1;
    }

    if (!emulator.selectModel(modelName)) {
        std::cerr << "Failed to select model: " << emulator.lastError() << "\n";
        return 1;
    }

    if (!emulator.start(opts.port)) {
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

void handleSignal(int) { g_stop.store(true); }

CliOptions parseArgs(int argc, char* argv[]) {
    CliOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            opts.showHelp = true;
            return opts;
        }

        if (arg == "-l" || arg == "--list") {
            opts.listModels = true;
            continue;
        }

        if (arg == "-m" || arg == "--model") {
            if (i + 1 >= argc) {
                opts.error = "Option '" + arg + "' requires an argument";
                return opts;
            }
            opts.modelId = argv[++i];
            continue;
        }

        if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc) {
                opts.error = "Option '" + arg + "' requires an argument";
                return opts;
            }

            const std::string value = argv[++i];
            const bool isNumber =
                !value.empty() &&
                std::all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; });

            if (!isNumber) {
                opts.error = "Invalid port: " + value;
                return opts;
            }

            try {
                const unsigned long port = std::stoul(value);
                if (port == 0 || port > std::numeric_limits<uint16_t>::max()) {
                    opts.error = "Invalid port: " + value;
                    return opts;
                }
                opts.port = static_cast<uint16_t>(port);
            } catch (const std::out_of_range&) {
                opts.error = "Invalid port: " + value;
                return opts;
            }
            continue;
        }

        opts.error = "Unknown option: " + arg;
        return opts;
    }

    return opts;
}

void printHelp(const char* progName) {
    std::cout << "Usage: " << progName << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  -l, --list           List available UPS models and exit\n"
              << "  -m, --model <ID>     Select UPS model by ID\n"
              << "  -p, --port <PORT>    Set UDP port (default: 161)\n"
              << "      --help           Show this help and exit\n";
}
