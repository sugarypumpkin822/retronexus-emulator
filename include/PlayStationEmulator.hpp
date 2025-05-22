#pragma once
#include "ConsoleEmulator.hpp"
#include <array>
#include <memory>

class PlayStationEmulator : public ConsoleEmulator {
public:
    PlayStationEmulator(ConsoleType type, const std::string& name, uint32_t ramSize);
    ~PlayStationEmulator() override = default;

    // Core emulation functions
    bool initialize() override;
    void step() override;
    void reset() override;
    bool loadROM(const std::vector<uint8_t>& data) override;

    // Memory management
    uint8_t readMemory(uint32_t address) const override;
    void writeMemory(uint32_t address, uint8_t value) override;

    // State management
    bool saveState(const std::string& filepath) override;
    bool loadState(const std::string& filepath) override;

    // Console specific information
    ConsoleType getConsoleType() const override { return consoleType; }
    std::string getConsoleName() const override { return consoleName; }
    
    // System requirements
    uint32_t getMinimumMemorySize() const override { return ramSize; }
    uint32_t getRecommendedMemorySize() const override { return ramSize * 2; }

protected:
    bool validateROM(const std::vector<uint8_t>& data) const override;
    bool detectConsoleType(const std::vector<uint8_t>& data) const override;

    // Memory regions
    std::vector<uint8_t> ram;        // Main RAM
    std::vector<uint8_t> vram;       // Video RAM
    std::vector<uint8_t> biosRom;    // BIOS ROM
    std::vector<uint8_t> gameRom;    // Game data

    // CPU state
    struct CPUState {
        uint32_t pc;     // Program Counter
        uint32_t hi, lo; // Multiply/Divide results
        std::array<uint32_t, 32> gpr;  // General Purpose Registers
        bool inDelaySlot;
    } cpu;

    // GPU state
    struct GPUState {
        uint32_t status;
        uint32_t control;
        std::vector<uint32_t> vram;
    } gpu;

private:
    ConsoleType consoleType;
    std::string consoleName;
    uint32_t ramSize;

    void initializeMemory();
    void initializeCPU();
    void initializeGPU();
    virtual void executeInstruction() = 0;
}; 
