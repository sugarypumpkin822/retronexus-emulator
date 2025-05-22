#pragma once
#include "PlayStationEmulator.hpp"

class PS1Emulator : public PlayStationEmulator {
public:
    PS1Emulator();
    ~PS1Emulator() override = default;

protected:
    bool validateROM(const std::vector<uint8_t>& data) const override;

private:
    // PS1 specific memory map
    static constexpr uint32_t RAM_SIZE = 2 * 1024 * 1024;  // 2MB
    static constexpr uint32_t VRAM_SIZE = 1 * 1024 * 1024; // 1MB
    static constexpr uint32_t BIOS_SIZE = 512 * 1024;      // 512KB

    // PS1 specific hardware
    struct {
        uint32_t status;
        uint32_t control;
    } cdrom;  // CD-ROM Controller

    // SPU memory map
    static constexpr uint32_t SPU_START = 0x1F801C00;
    static constexpr uint32_t SPU_END = 0x1F802000;
    static constexpr uint32_t SPU_RAM_START = 0x1F801C00;
    static constexpr uint32_t SPU_VOICE_START = 0x1F801C00;
    static constexpr uint32_t SPU_CONTROL_START = 0x1F801D80;
    static constexpr uint32_t SPU_STATUS_START = 0x1F801D88;
    static constexpr uint32_t SPU_RAM_SIZE = 512 * 1024;  // 512KB SPU RAM

    void executeInstruction() override;
    void handleSPUOperation();
    void handleCDROMOperation();
    void updateSPUStatus();
}; 
