#include "PS2Emulator.hpp"
#include <iostream>

PS2Emulator::PS2Emulator()
    : PlayStationEmulator(ConsoleType::PS2, "Sony PlayStation 2", RAM_SIZE) {
    ee = {};
    gs = {};
    iop = {};
    gs.local_mem.resize(4 * 1024 * 1024, 0); // 4MB GS memory
}

bool PS2Emulator::validateROM(const std::vector<uint8_t>& data) const {
    // Check minimum size
    if (data.size() < 0x800) {
        return false;
    }

    // Check for PS2 specific identifiers
    // PS2 ISO files typically start with these bytes
    static const uint8_t PS2_MAGIC[] = {0x50, 0x53, 0x32, 0x4D}; // "PS2M"
    for (int i = 0; i < 4; i++) {
        if (data[i] != PS2_MAGIC[i]) {
            return false;
        }
    }

    return true;
}

void PS2Emulator::executeInstruction() {
    // Execute one instruction on each processor
    executeEEInstruction();
    executeIOPInstruction();
    handleGSOperation();
    handleSPU2Operation();
}

void PS2Emulator::executeEEInstruction() {
    if (!ee.pc) return;

    // Fetch instruction from EE memory
    uint64_t instruction = 
        (static_cast<uint64_t>(readMemory(ee.pc)) << 56) |
        (static_cast<uint64_t>(readMemory(ee.pc + 1)) << 48) |
        (static_cast<uint64_t>(readMemory(ee.pc + 2)) << 40) |
        (static_cast<uint64_t>(readMemory(ee.pc + 3)) << 32) |
        (static_cast<uint64_t>(readMemory(ee.pc + 4)) << 24) |
        (static_cast<uint64_t>(readMemory(ee.pc + 5)) << 16) |
        (static_cast<uint64_t>(readMemory(ee.pc + 6)) << 8) |
        static_cast<uint64_t>(readMemory(ee.pc + 7));

    ee.pc += 8;

    // Decode and execute EE instruction
    uint8_t opcode = instruction >> 58;
    uint8_t rs = (instruction >> 53) & 0x1F;
    uint8_t rt = (instruction >> 48) & 0x1F;
    uint8_t rd = (instruction >> 43) & 0x1F;
    uint8_t shamt = (instruction >> 38) & 0x1F;
    uint8_t funct = instruction & 0x3F;

    switch (opcode) {
        case 0x00: // SPECIAL
            switch (funct) {
                case 0x00: // SLL
                    if (rd != 0) {
                        ee.gpr[rd] = ee.gpr[rt] << shamt;
                    }
                    break;
                // Add more EE SPECIAL instructions
                default:
                    std::cerr << "Unhandled EE SPECIAL instruction: " << std::hex << funct << std::endl;
            }
            break;
        // Add more EE instructions
        default:
            std::cerr << "Unknown EE opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
    }
}

void PS2Emulator::executeIOPInstruction() {
    if (!iop.pc) return;

    // Fetch instruction from IOP memory
    uint32_t instruction = 
        (readMemory(iop.pc) << 24) |
        (readMemory(iop.pc + 1) << 16) |
        (readMemory(iop.pc + 2) << 8) |
        readMemory(iop.pc + 3);

    iop.pc += 4;

    // Decode and execute IOP instruction (similar to PS1 CPU)
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
                    if (rd != 0) {
                        iop.gpr[rd] = iop.gpr[rt] << shamt;
                    }
                    break;
                // Add more IOP SPECIAL instructions
                default:
                    std::cerr << "Unhandled IOP SPECIAL instruction: " << std::hex << funct << std::endl;
            }
            break;
        // Add more IOP instructions
        default:
            std::cerr << "Unknown IOP opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
    }
}

void PS2Emulator::handleGSOperation() {
    // TODO: Implement Graphics Synthesizer operations
    // This would handle 3D rendering and 2D graphics
    if (gs.status & 0x1) {  // If GS is busy
        // Process current graphics command
        // Update frame buffer
        // Handle VSync
    }
}

void PS2Emulator::handleSPU2Operation() {
    // Check if current instruction accesses SPU2 memory range
    if (iop.pc >= SPU2_START && iop.pc < SPU2_END) {
        updateSPU2Status();

        // Process both SPU2 cores
        processSPU2Core(SPU2_CORE0_START);
        processSPU2Core(SPU2_CORE1_START);

        // Mix output from both cores
        mixSPU2Output();
    }
}

void PS2Emulator::processSPU2Core(uint32_t coreBase) {
    if (!spu) return;

    // Each core has 24 voices (same as PS1)
    for (uint8_t voice = 0; voice < 24; ++voice) {
        uint32_t voiceBase = coreBase + voice * 16;
        
        // Read voice parameters through memory mapped registers
        uint16_t voiceControl = spu->read((voiceBase - SPU2_START) / 2);
        
        if (voiceControl & 0x8000) {  // Voice is keyed on
            // Process voice parameters (volume, pitch, etc.)
            uint16_t volume = spu->read((voiceBase + 2 - SPU2_START) / 2);
            uint16_t pitch = spu->read((voiceBase + 4 - SPU2_START) / 2);
            uint32_t addr = spu->read((voiceBase + 6 - SPU2_START) / 2) << 3;  // Sample address

            // Update voice parameters in SPU
            spu->write((voiceBase + 2 - SPU2_START) / 2, volume);
            spu->write((voiceBase + 4 - SPU2_START) / 2, pitch);
            spu->write((voiceBase + 6 - SPU2_START) / 2, addr >> 3);
        }
    }
}

void PS2Emulator::updateSPU2Status() {
    if (!spu) return;

    // Update status for both cores
    for (uint32_t core = 0; core < 2; ++core) {
        uint32_t coreBase = (core == 0) ? SPU2_CORE0_START : SPU2_CORE1_START;
        
        // Check if any voices are active in this core
        bool anyVoiceActive = false;
        for (uint8_t voice = 0; voice < 24; ++voice) {
            uint32_t voiceBase = coreBase + voice * 16;
            uint16_t voiceControl = spu->read((voiceBase - SPU2_START) / 2);
            if (voiceControl & 0x8000) {
                anyVoiceActive = true;
                break;
            }
        }

        // Update core status
        uint32_t statusAddr = coreBase + 0x344;  // Status register offset
        uint16_t status = spu->read((statusAddr - SPU2_START) / 2);
        status &= ~0x0080;  // Clear busy flag
        if (anyVoiceActive) {
            status |= 0x0080;  // Set busy flag if any voice is active
        }
        spu->write((statusAddr - SPU2_START) / 2, status);
    }
}

void PS2Emulator::mixSPU2Output() {
    if (!spu) return;

    // Get audio buffers from both cores
    const std::vector<int16_t>& audioBuffer = spu->getAudioBuffer();
    if (audioBuffer.empty()) return;

    // Mix the stereo output from both cores
    // In PS2, each core can output stereo audio
    // Core 0 typically handles main audio
    // Core 1 typically handles effects or additional channels
    
    // Clear the audio buffer after processing to prepare for next frame
    spu->clearAudioBuffer();
} 
