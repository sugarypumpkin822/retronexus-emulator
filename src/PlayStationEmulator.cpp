#include "PlayStationEmulator.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

PlayStationEmulator::PlayStationEmulator(ConsoleType type, const std::string& name, uint32_t ramSize)
    : consoleType(type), consoleName(name), ramSize(ramSize) {
    reset();
}

bool PlayStationEmulator::initialize() {
    reset();
    return true;
}

void PlayStationEmulator::step() {
    executeInstruction();
    
    // Step the SPU
    if (spu) {
        spu->step();
    }
}

void PlayStationEmulator::reset() {
    initializeMemory();
    initializeCPU();
    initializeGPU();
    initializeSPU();
}

bool PlayStationEmulator::loadROM(const std::vector<uint8_t>& data) {
    if (!validateROM(data)) {
        return false;
    }

    gameRom = data;
    return true;
}

uint8_t PlayStationEmulator::readMemory(uint32_t address) const {
    // Basic memory map implementation
    if (address < ram.size()) {
        return ram[address];
    }
    else if (address >= 0x1F000000 && address < 0x1F800000) {
        // BIOS ROM
        uint32_t biosAddr = address - 0x1F000000;
        if (biosAddr < biosRom.size()) {
            return biosRom[biosAddr];
        }
    }
    else if (address >= 0x80000000 && address < 0x80000000 + ram.size()) {
        // Mirror of RAM in kernel space
        return ram[address - 0x80000000];
    }
    else if (address >= 0x1F801C00 && address < 0x1F802000) {
        // SPU registers
        if (spu) {
            return static_cast<uint8_t>(spu->read((address - 0x1F801C00) / 2) >> ((address & 1) * 8));
        }
    }

    std::cerr << "Memory read from unhandled address: 0x" << std::hex << address << std::endl;
    return 0;
}

void PlayStationEmulator::writeMemory(uint32_t address, uint8_t value) {
    // Basic memory map implementation
    if (address < ram.size()) {
        ram[address] = value;
    }
    else if (address >= 0x80000000 && address < 0x80000000 + ram.size()) {
        // Mirror of RAM in kernel space
        ram[address - 0x80000000] = value;
    }
    else if (address >= 0x1F801C00 && address < 0x1F802000) {
        // SPU registers
        if (spu) {
            static uint16_t spuWord = 0;
            if (address & 1) {
                spuWord = (spuWord & 0xFF) | (value << 8);
                spu->write((address - 0x1F801C00) / 2, spuWord);
            } else {
                spuWord = (spuWord & 0xFF00) | value;
            }
        }
    }
    else {
        std::cerr << "Memory write to unhandled address: 0x" << std::hex << address << std::endl;
    }
}

bool PlayStationEmulator::saveState(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        return false;
    }

    // Save RAM
    file.write(reinterpret_cast<const char*>(ram.data()), ram.size());
    
    // Save CPU state
    file.write(reinterpret_cast<const char*>(&cpu), sizeof(cpu));
    
    // Save GPU state
    file.write(reinterpret_cast<const char*>(&gpu), sizeof(gpu));
    
    // Save SPU state
    if (spu) {
        std::string spuStatePath = filepath + ".spu";
        if (!spu->saveState(spuStatePath)) {
            return false;
        }
    }
    
    return true;
}

bool PlayStationEmulator::loadState(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return false;
    }

    // Load RAM
    file.read(reinterpret_cast<char*>(ram.data()), ram.size());
    
    // Load CPU state
    file.read(reinterpret_cast<char*>(&cpu), sizeof(cpu));
    
    // Load GPU state
    file.read(reinterpret_cast<char*>(&gpu), sizeof(gpu));
    
    // Load SPU state
    if (spu) {
        std::string spuStatePath = filepath + ".spu";
        if (!spu->loadState(spuStatePath)) {
            return false;
        }
    }
    
    return true;
}

bool PlayStationEmulator::validateROM(const std::vector<uint8_t>& data) const {
    // Basic size check
    if (data.size() < 0x800) {
        return false;
    }

    // Check for PlayStation magic values
    return (data[0] == 'P' && data[1] == 'S' && data[2] == 'X' && data[3] == ' ');
}

bool PlayStationEmulator::detectConsoleType(const std::vector<uint8_t>& data) const {
    return validateROM(data);
}

void PlayStationEmulator::initializeMemory() {
    ram.resize(ramSize, 0);
    vram.resize(1024 * 1024, 0);  // 1MB VRAM
    biosRom.resize(512 * 1024, 0); // 512KB BIOS
}

void PlayStationEmulator::initializeCPU() {
    cpu = {};  // Zero initialize
    cpu.pc = 0xBFC00000;  // BIOS entry point
}

void PlayStationEmulator::initializeGPU() {
    gpu = {};  // Zero initialize
    gpu.vram.resize(1024 * 1024, 0);  // 1MB VRAM
}

void PlayStationEmulator::initializeSPU() {
    // Create SPU with appropriate mode based on console type
    bool isPS2 = (consoleType == ConsoleType::PS2);
    spu = std::make_unique<SPU>(isPS2);
    spu->initialize();
} 
