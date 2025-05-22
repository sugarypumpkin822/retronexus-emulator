#pragma once
#include "PlayStationEmulator.hpp"

class PS2Emulator : public PlayStationEmulator {
public:
    PS2Emulator();
    ~PS2Emulator() override = default;

protected:
    bool validateROM(const std::vector<uint8_t>& data) const override;

private:
    // PS2 specific memory map
    static constexpr uint32_t RAM_SIZE = 32 * 1024 * 1024;   // 32MB
    static constexpr uint32_t VRAM_SIZE = 4 * 1024 * 1024;   // 4MB
    static constexpr uint32_t BIOS_SIZE = 4 * 1024 * 1024;   // 4MB

    // SPU2 memory map
    static constexpr uint32_t SPU2_START = 0x1F900000;
    static constexpr uint32_t SPU2_END = 0x1F900800;
    static constexpr uint32_t SPU2_CORE0_START = 0x1F900000;
    static constexpr uint32_t SPU2_CORE1_START = 0x1F900400;
    static constexpr uint32_t SPU2_RAM_SIZE = 2 * 1024 * 1024;  // 2MB SPU2 RAM

    // Emotion Engine (Main CPU)
    struct EmotionEngine {
        uint64_t gpr[32];    // General Purpose Registers
        uint64_t pc;         // Program Counter
        uint64_t hi, lo;     // Multiply/Divide results
    } ee;

    // Graphics Synthesizer
    struct GraphicsSynthesizer {
        uint32_t status;
        uint32_t control;
        std::vector<uint32_t> local_mem;  // GS local memory
    } gs;

    // I/O Processor
    struct IOP {
        uint32_t gpr[32];    // General Purpose Registers
        uint32_t pc;         // Program Counter
        uint32_t hi, lo;     // Multiply/Divide results
    } iop;

    void executeInstruction() override;
    void executeEEInstruction();
    void executeIOPInstruction();
    void handleGSOperation();
    void handleSPU2Operation();
    void updateSPU2Status();

    // SPU2 helper functions
    void processSPU2Core(uint32_t coreBase);
    void mixSPU2Output();
}; 
