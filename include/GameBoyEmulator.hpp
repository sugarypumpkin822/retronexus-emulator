#pragma once
#include "ConsoleEmulator.hpp"
#include <array>

class GameBoyEmulator : public ConsoleEmulator {
public:
    GameBoyEmulator();
    ~GameBoyEmulator() override = default;

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
    ConsoleType getConsoleType() const override { return ConsoleType::GAMEBOY; }
    std::string getConsoleName() const override { return "Nintendo Game Boy"; }
    
    // System requirements
    uint32_t getMinimumMemorySize() const override { return 32 * 1024; } // 32KB
    uint32_t getRecommendedMemorySize() const override { return 64 * 1024; } // 64KB

protected:
    bool validateROM(const std::vector<uint8_t>& data) const override;
    bool detectConsoleType(const std::vector<uint8_t>& data) const override;

private:
    // GameBoy specific memory regions
    std::array<uint8_t, 0x10000> memory;  // 64KB total memory
    std::vector<uint8_t> cartridgeROM;    // Cartridge ROM data
    
    // CPU registers
    struct {
        uint8_t a, f;  // Accumulator & flags
        uint8_t b, c;  // BC pair
        uint8_t d, e;  // DE pair
        uint8_t h, l;  // HL pair
        uint16_t sp;   // Stack pointer
        uint16_t pc;   // Program counter
    } registers;

    // GPU registers
    struct {
        uint8_t lcdc;  // LCD Control
        uint8_t stat;  // LCD Status
        uint8_t ly;    // LCD Y-Coordinate
    } gpu;

    void initializeRegisters();
    void executeInstruction();
}; 
