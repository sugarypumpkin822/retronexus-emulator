#include "Emulator.hpp"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Usage: emulator <filename>\n";
    std::cout << "Supports loading any file type for emulation\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage();
        return 1;
    }

    try {
        Emulator emu;
        std::string filepath = argv[1];

        std::cout << "Initializing emulator...\n";
        emu.initialize();

        std::cout << "Loading file: " << filepath << "\n";
        if (!emu.loadFile(filepath)) {
            std::cerr << "Failed to load file\n";
            return 1;
        }

        std::cout << "Starting emulation...\n";
        emu.run();

        std::cout << "Emulation completed\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
} 
