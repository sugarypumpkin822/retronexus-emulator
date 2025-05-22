#include "PS1Emulator.hpp"
#include <iostream>

PS1Emulator::PS1Emulator()
    : PlayStationEmulator(ConsoleType::PS1, "Sony PlayStation", RAM_SIZE) {
    cdrom = {};
}

bool PS1Emulator::validateROM(const std::vector<uint8_t>& data) const {
    // Check minimum size
    if (data.size() < 0x800) {
        return false;
    }

    // Check for PS1 specific identifiers
    // PS1 games typically start with "PS-X EXE"
    const char* PS1_MAGIC = "PS-X EXE";
    for (int i = 0; i < 8; i++) {
        if (data[i] != PS1_MAGIC[i]) {
            return false;
        }
    }

    return true;
}

void PS1Emulator::executeInstruction() {
    if (!cpu.pc) return;

    // Fetch instruction
    uint32_t instruction = 
        (readMemory(cpu.pc) << 24) |
        (readMemory(cpu.pc + 1) << 16) |
        (readMemory(cpu.pc + 2) << 8) |
        readMemory(cpu.pc + 3);
    
    cpu.pc += 4;

    // Decode and execute
    uint8_t opcode = instruction >> 26;
    uint8_t rs = (instruction >> 21) & 0x1F;
    uint8_t rt = (instruction >> 16) & 0x1F;
    uint8_t rd = (instruction >> 11) & 0x1F;
    uint8_t shamt = (instruction >> 6) & 0x1F;
    uint8_t funct = instruction & 0x3F;

    switch (opcode) {
        case 0x00: // SPECIAL
            switch (funct) {
                case 0x00: // SLL
                    if (rd != 0) { // R0 is always 0
                        cpu.gpr[rd] = cpu.gpr[rt] << shamt;
                    }
                    break;
                case 0x02: // SRL
                    if (rd != 0) {
                        cpu.gpr[rd] = cpu.gpr[rt] >> shamt;
                    }
                    break;
                // Add more SPECIAL instructions as needed
                default:
                    std::cerr << "Unhandled SPECIAL instruction: " << std::hex << funct << std::endl;
            }
            break;

        case 0x02: // J - Jump
            cpu.pc = (cpu.pc & 0xF0000000) | ((instruction & 0x3FFFFFF) << 2);
            break;

        case 0x08: // ADDI
            if (rt != 0) {
                int32_t imm = static_cast<int16_t>(instruction & 0xFFFF);
                cpu.gpr[rt] = cpu.gpr[rs] + imm;
            }
            break;

        // Add more instructions as needed

        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
    }

    // Handle hardware components
    handleSPUOperation();
    handleCDROMOperation();
}

void PS1Emulator::handleSPUOperation() {
    // Check if the current instruction accesses SPU memory range
    if (cpu.pc >= SPU_START && cpu.pc < SPU_END) {
        updateSPUStatus();
        
        // Handle voice parameters
        if (cpu.pc >= SPU_VOICE_START && cpu.pc < SPU_CONTROL_START) {
            uint8_t voice = (cpu.pc - SPU_VOICE_START) / 16;  // Each voice has 16 bytes of registers
            if (voice < 24) {  // PS1 has 24 voices
                // Voice parameters are automatically handled by the SPU class
                // through memory mapped registers
            }
        }
        // Handle control registers
        else if (cpu.pc >= SPU_CONTROL_START && cpu.pc < SPU_STATUS_START) {
            // Control registers are handled through memory mapping in the base class
        }
    }
}

void PS1Emulator::handleCDROMOperation() {
    // TODO: Implement CD-ROM operations
    // This would handle reading from the game disc
}

void PS1Emulator::updateSPUStatus() {
    // Update SPU status based on current state
    if (spu) {
        // Check if any voices are active
        bool anyVoiceActive = false;
        for (uint8_t i = 0; i < 24; ++i) {
            // Read voice status through memory mapped registers
            uint16_t voiceControl = spu->read((SPU_VOICE_START - SPU_START + i * 16) / 2);
            if (voiceControl & 0x8000) {  // Check if voice is keyed on
                anyVoiceActive = true;
                break;
            }
        }

        // Update status register
        uint16_t status = spu->read((SPU_STATUS_START - SPU_START) / 2);
        status &= ~0x0040;  // Clear busy flag
        if (anyVoiceActive) {
            status |= 0x0040;  // Set busy flag if any voice is active
        }
        spu->write((SPU_STATUS_START - SPU_START) / 2, status);
    }
} 
