#include "Emulator.hpp"
#include "GameBoyEmulator.hpp"
#include <iostream>
#include <algorithm>

Emulator::Emulator() : isRunning(false), console(nullptr) {
}

Emulator::~Emulator() {
    stop();
}

bool Emulator::setConsoleType(ConsoleType type) {
    console = createConsoleEmulator(type);
    return console != nullptr;
}

ConsoleType Emulator::getConsoleType() const {
    return console ? console->getConsoleType() : ConsoleType::UNKNOWN;
}

std::string Emulator::getConsoleName() const {
    return console ? console->getConsoleName() : "Unknown";
}

bool Emulator::loadFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // Read the entire file into fileData
    fileData.clear();
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    fileData.resize(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

    // Auto-detect console type if not set
    if (!console) {
        ConsoleType detectedType = detectConsoleType(fileData);
        if (!setConsoleType(detectedType)) {
            std::cerr << "Failed to create emulator for detected console type" << std::endl;
            return false;
        }
    }

    return console->loadROM(fileData);
}

void Emulator::writeMemory(uint32_t address, uint8_t value) {
    if (console) {
        console->writeMemory(address, value);
    }
}

uint8_t Emulator::readMemory(uint32_t address) const {
    if (console) {
        return console->readMemory(address);
    }
    throw std::runtime_error("No console emulator initialized");
}

void Emulator::initialize() {
    if (console) {
        console->initialize();
        isRunning = false;
    }
}

void Emulator::step() {
    if (console && isRunning) {
        console->step();
    }
}

void Emulator::run() {
    if (console) {
        isRunning = true;
        while (isRunning) {
            step();
        }
    }
}

void Emulator::stop() {
    isRunning = false;
}

void Emulator::reset() {
    if (console) {
        console->reset();
        isRunning = false;
    }
}

void Emulator::saveState(const std::string& filepath) {
    if (console) {
        console->saveState(filepath);
    }
}

void Emulator::loadState(const std::string& filepath) {
    if (console) {
        console->loadState(filepath);
    }
}

ConsoleType Emulator::detectConsoleType(const std::vector<uint8_t>& data) const {
    // TODO: Implement ROM header detection for different console types
    // This is a placeholder implementation
    if (data.size() < 4) return ConsoleType::UNKNOWN;

    // Example detection based on common ROM headers
    if (data.size() >= 0x150 && data[0x104] == 0xCE && data[0x105] == 0xED) {
        return ConsoleType::GAMEBOY;
    }
    
    if (data.size() >= 0x200 && data[0x1B] == 0x53 && data[0x1A] == 0x45) {
        return ConsoleType::GENESIS;
    }

    // Add more console detection logic here
    
    return ConsoleType::UNKNOWN;
}

std::unique_ptr<ConsoleEmulator> Emulator::createConsoleEmulator(ConsoleType type) const {
    switch (type) {
        case ConsoleType::GAMEBOY:
            return std::make_unique<GameBoyEmulator>();
        // Add more console types here as they are implemented
        default:
            std::cerr << "Unsupported console type" << std::endl;
            return nullptr;
    }
} 
