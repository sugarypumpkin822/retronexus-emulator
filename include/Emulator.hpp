#pragma once
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <unordered_map>
#include "ConsoleType.hpp"
#include "ConsoleEmulator.hpp"

class Emulator {
public:
    Emulator();
    ~Emulator();

    // Console management
    bool setConsoleType(ConsoleType type);
    ConsoleType getConsoleType() const;
    std::string getConsoleName() const;

    // File loading
    bool loadFile(const std::string& filepath);
    
    // Memory management
    void writeMemory(uint32_t address, uint8_t value);
    uint8_t readMemory(uint32_t address) const;
    
    // Emulation control
    void initialize();
    void step();
    void run();
    void stop();
    void reset();

    // State management
    void saveState(const std::string& filepath);
    void loadState(const std::string& filepath);

private:
    // Console emulator instance
    std::unique_ptr<ConsoleEmulator> console;
    
    // Emulation state
    bool isRunning;
    std::vector<uint8_t> fileData;

    // Helper functions
    ConsoleType detectConsoleType(const std::vector<uint8_t>& data) const;
    std::unique_ptr<ConsoleEmulator> createConsoleEmulator(ConsoleType type) const;
}; 
