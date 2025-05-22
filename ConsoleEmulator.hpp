#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include "ConsoleType.hpp"

class ConsoleEmulator {
public:
    virtual ~ConsoleEmulator() = default;

    // Core emulation functions
    virtual bool initialize() = 0;
    virtual void step() = 0;
    virtual void reset() = 0;
    virtual bool loadROM(const std::vector<uint8_t>& data) = 0;

    // Memory management
    virtual uint8_t readMemory(uint32_t address) const = 0;
    virtual void writeMemory(uint32_t address, uint8_t value) = 0;

    // State management
    virtual bool saveState(const std::string& filepath) = 0;
    virtual bool loadState(const std::string& filepath) = 0;

    // Console specific information
    virtual ConsoleType getConsoleType() const = 0;
    virtual std::string getConsoleName() const = 0;
    
    // System requirements
    virtual uint32_t getMinimumMemorySize() const = 0;
    virtual uint32_t getRecommendedMemorySize() const = 0;

protected:
    // Common utility functions
    virtual bool validateROM(const std::vector<uint8_t>& data) const = 0;
    virtual bool detectConsoleType(const std::vector<uint8_t>& data) const = 0;
}; 
