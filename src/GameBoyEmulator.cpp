#include "GameBoyEmulator.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

GameBoyEmulator::GameBoyEmulator() {
    reset();
}

bool GameBoyEmulator::initialize() {
    reset();
    return true;
}

void GameBoyEmulator::step() {
    executeInstruction();
}

void GameBoyEmulator::reset() {
    std::fill(memory.begin(), memory.end(), 0);
    initializeRegisters();
}

bool GameBoyEmulator::loadROM(const std::vector<uint8_t>& data) {
    if (!validateROM(data)) {
        return false;
    }

    cartridgeROM = data;
    // Copy ROM to memory (first 32KB)
    size_t romSize = std::min(data.size(), static_cast<size_t>(0x8000));
    std::copy(data.begin(), data.begin() + romSize, memory.begin());
    return true;
}

uint8_t GameBoyEmulator::readMemory(uint32_t address) const {
    if (address >= memory.size()) {
        throw std::out_of_range("Memory address out of bounds");
    }
    return memory[address];
}

void GameBoyEmulator::writeMemory(uint32_t address, uint8_t value) {
    if (address >= memory.size()) {
        throw std::out_of_range("Memory address out of bounds");
    }
    
    // Handle memory regions
    if (address < 0x8000) {
        // ROM area - ignore writes
        return;
    }
    
    memory[address] = value;
}

bool GameBoyEmulator::saveState(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        return false;
    }

    // Save memory
    file.write(reinterpret_cast<const char*>(memory.data()), memory.size());
    
    // Save registers
    file.write(reinterpret_cast<const char*>(&registers), sizeof(registers));
    file.write(reinterpret_cast<const char*>(&gpu), sizeof(gpu));
    
    return true;
}

bool GameBoyEmulator::loadState(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return false;
    }

    // Load memory
    file.read(reinterpret_cast<char*>(memory.data()), memory.size());
    
    // Load registers
    file.read(reinterpret_cast<char*>(&registers), sizeof(registers));
    file.read(reinterpret_cast<char*>(&gpu), sizeof(gpu));
    
    return true;
}

bool GameBoyEmulator::validateROM(const std::vector<uint8_t>& data) const {
    // Check minimum size
    if (data.size() < 0x150) {
        return false;
    }

    // Check Nintendo logo
    static const uint8_t NINTENDO_LOGO[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D
    };
    
    return std::memcmp(&data[0x104], NINTENDO_LOGO, sizeof(NINTENDO_LOGO)) == 0;
}

bool GameBoyEmulator::detectConsoleType(const std::vector<uint8_t>& data) const {
    return validateROM(data);
}

void GameBoyEmulator::initializeRegisters() {
    registers = {};  // Zero initialize
    registers.a = 0x01;
    registers.f = 0xB0;
    registers.b = 0x00;
    registers.c = 0x13;
    registers.d = 0x00;
    registers.e = 0xD8;
    registers.h = 0x01;
    registers.l = 0x4D;
    registers.sp = 0xFFFE;
    registers.pc = 0x0100;

    gpu = {};  // Zero initialize
}

void GameBoyEmulator::executeInstruction() {
    // Fetch
    uint8_t opcode = readMemory(registers.pc++);
    
    // Decode and execute
    switch (opcode) {
        // TODO: Implement GameBoy CPU instructions
        case 0x00:  // NOP
            break;
        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
            break;
    }
} 
