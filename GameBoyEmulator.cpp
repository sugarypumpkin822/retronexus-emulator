#include "GameBoyEmulator.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <SDL2/SDL.h>

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
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, nn
            registers.bc = readWord(registers.pc);
            registers.pc += 2;
            break;
        case 0x02: // LD (BC), A
            writeMemory(registers.bc, registers.a);
            break;
        case 0x03: // INC BC
            registers.bc++;
            break;
        case 0x04: // INC B
            registers.b = inc(registers.b);
            break;
        case 0x05: // DEC B
            registers.b = dec(registers.b);
            break;
        case 0x06: // LD B, n
            registers.b = readMemory(registers.pc++);
            break;
        case 0x07: // RLCA
            registers.a = rlc(registers.a);
            break;
        case 0x08: // LD (nn), SP
            writeWord(readWord(registers.pc), registers.sp);
            registers.pc += 2;
            break;
        case 0x09: // ADD HL, BC
            registers.hl = add16(registers.hl, registers.bc);
            break;
        case 0x0A: // LD A, (BC)
            registers.a = readMemory(registers.bc);
            break;
        case 0x0B: // DEC BC
            registers.bc--;
            break;
        case 0x0C: // INC C
            registers.c = inc(registers.c);
            break;
        case 0x0D: // DEC C
            registers.c = dec(registers.c);
            break;
        case 0x0E: // LD C, n
            registers.c = readMemory(registers.pc++);
            break;
        case 0x0F: // RRCA
            registers.a = rrc(registers.a);
            break;
        case 0x10: // STOP
            // TODO: Implement STOP instruction
            break;
        case 0x11: // LD DE, nn
            registers.de = readWord(registers.pc);
            registers.pc += 2;
            break;
        case 0x12: // LD (DE), A
            writeMemory(registers.de, registers.a);
            break;
        case 0x13: // INC DE
            registers.de++;
            break;
        case 0x14: // INC D
            registers.d = inc(registers.d);
            break;
        case 0x15: // DEC D
            registers.d = dec(registers.d);
            break;
        case 0x16: // LD D, n
            registers.d = readMemory(registers.pc++);
            break;
        case 0x17: // RLA
            registers.a = rl(registers.a);
            break;
        case 0x18: // JR n
            registers.pc += static_cast<int8_t>(readMemory(registers.pc)) + 1;
            break;
        case 0x19: // ADD HL, DE
            registers.hl = add16(registers.hl, registers.de);
            break;
        case 0x1A: // LD A, (DE)
            registers.a = readMemory(registers.de);
            break;
        case 0x1B: // DEC DE
            registers.de--;
            break;
        case 0x1C: // INC E
            registers.e = inc(registers.e);
            break;
        case 0x1D: // DEC E
            registers.e = dec(registers.e);
            break;
        case 0x1E: // LD E, n
            registers.e = readMemory(registers.pc++);
            break;
        case 0x1F: // RRA
            registers.a = rr(registers.a);
            break;
        case 0x20: // JR NZ, n
            if (!getZeroFlag()) {
                registers.pc += static_cast<int8_t>(readMemory(registers.pc)) + 1;
            } else {
                registers.pc++;
            }
            break;
        case 0x21: // LD HL, nn
            registers.hl = readWord(registers.pc);
            registers.pc += 2;
            break;
        case 0x22: // LD (HL+), A
            writeMemory(registers.hl++, registers.a);
            break;
        case 0x23: // INC HL
            registers.hl++;
            break;
        case 0x24: // INC H
            registers.h = inc(registers.h);
            break;
        case 0x25: // DEC H
            registers.h = dec(registers.h);
            break;
        case 0x26: // LD H, n
            registers.h = readMemory(registers.pc++);
            break;
        case 0x27: // DAA
            daa();
            break;
        case 0x28: // JR Z, n
            if (getZeroFlag()) {
                registers.pc += static_cast<int8_t>(readMemory(registers.pc)) + 1;
            } else {
                registers.pc++;
            }
            break;
        case 0x29: // ADD HL, HL
            registers.hl = add16(registers.hl, registers.hl);
            break;
        case 0x2A: // LD A, (HL+)
            registers.a = readMemory(registers.hl++);
            break;
        case 0x2B: // DEC HL
            registers.hl--;
            break;
        case 0x2C: // INC L
            registers.l = inc(registers.l);
            break;
        case 0x2D: // DEC L
            registers.l = dec(registers.l);
            break;
        case 0x2E: // LD L, n
            registers.l = readMemory(registers.pc++);
            break;
        case 0x2F: // CPL
            registers.a = ~registers.a;
            setSubtractFlag(true);
            setHalfCarryFlag(true);
            break;
        case 0x30: // JR NC, n
            if (!getCarryFlag()) {
                registers.pc += static_cast<int8_t>(readMemory(registers.pc)) + 1;
            } else {
                registers.pc++;
            }
            break;
        case 0x31: // LD SP, nn
            registers.sp = readWord(registers.pc);
            registers.pc += 2;
            break;
        case 0x32: // LD (HL-), A
            writeMemory(registers.hl--, registers.a);
            break;
        case 0x33: // INC SP
            registers.sp++;
            break;
        case 0x34: // INC (HL)
            writeMemory(registers.hl, inc(readMemory(registers.hl)));
            break;
        case 0x35: // DEC (HL)
            writeMemory(registers.hl, dec(readMemory(registers.hl)));
            break;
        case 0x36: // LD (HL), n
            writeMemory(registers.hl, readMemory(registers.pc++));
            break;
        case 0x37: // SCF
            setCarryFlag(true);
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        case 0x38: // JR C, n
            if (getCarryFlag()) {
                registers.pc += static_cast<int8_t>(readMemory(registers.pc)) + 1;
            } else {
                registers.pc++;
            }
            break;
        case 0x39: // ADD HL, SP
            registers.hl = add16(registers.hl, registers.sp);
            break;
        case 0x3A: // LD A, (HL-)
            registers.a = readMemory(registers.hl--);
            break;
        case 0x3B: // DEC SP
            registers.sp--;
            break;
        case 0x3C: // INC A
            registers.a = inc(registers.a);
            break;
        case 0x3D: // DEC A
            registers.a = dec(registers.a);
            break;
        case 0x3E: // LD A, n
            registers.a = readMemory(registers.pc++);
            break;
        case 0x3F: // CCF
            setCarryFlag(!getCarryFlag());
            setSubtractFlag(false);
            setHalfCarryFlag(false);
            break;
        case 0x40: // LD B, B
            registers.b = registers.b;
            break;
        case 0x41: // LD B, C
            registers.b = registers.c;
            break;
        case 0x42: // LD B, D
            registers.b = registers.d;
            break;
        case 0x43: // LD B, E
            registers.b = registers.e;
            break;
        case 0x44: // LD B, H
            registers.b = registers.h;
            break;
        case 0x45: // LD B, L
            registers.b = registers.l;
            break;
        case 0x46: // LD B, (HL)
            registers.b = readMemory(registers.hl);
            break;
        case 0x47: // LD B, A
            registers.b = registers.a;
            break;
        case 0x48: // LD C, B
            registers.c = registers.b;
            break;
        case 0x49: // LD C, C
            registers.c = registers.c;
            break;
        case 0x4A: // LD C, D
            registers.c = registers.d;
            break;
        case 0x4B: // LD C, E
            registers.c = registers.e;
            break;
        case 0x4C: // LD C, H
            registers.c = registers.h;
            break;
        case 0x4D: // LD C, L
            registers.c = registers.l;
            break;
        case 0x4E: // LD C, (HL)
            registers.c = readMemory(registers.hl);
            break;
        case 0x4F: // LD C, A
            registers.c = registers.a;
            break;
        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
            break;
    }
}

// CPU Operations
uint8_t GameBoyEmulator::inc(uint8_t value) {
    uint8_t result = value + 1;
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag((value & 0x0F) == 0x0F);
    return result;
}

uint8_t GameBoyEmulator::dec(uint8_t value) {
    uint8_t result = value - 1;
    setZeroFlag(result == 0);
    setSubtractFlag(true);
    setHalfCarryFlag((value & 0x0F) == 0);
    return result;
}

uint8_t GameBoyEmulator::rlc(uint8_t value) {
    uint8_t result = (value << 1) | (value >> 7);
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(value & 0x80);
    return result;
}

uint8_t GameBoyEmulator::rrc(uint8_t value) {
    uint8_t result = (value >> 1) | (value << 7);
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(value & 0x01);
    return result;
}

uint16_t GameBoyEmulator::add16(uint16_t a, uint16_t b) {
    uint32_t result = a + b;
    setSubtractFlag(false);
    setHalfCarryFlag(((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF);
    setCarryFlag(result > 0xFFFF);
    return static_cast<uint16_t>(result);
}

// Additional CPU Operations
uint8_t GameBoyEmulator::rl(uint8_t value) {
    uint8_t result = (value << 1) | (getCarryFlag() ? 1 : 0);
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(value & 0x80);
    return result;
}

uint8_t GameBoyEmulator::rr(uint8_t value) {
    uint8_t result = (value >> 1) | (getCarryFlag() ? 0x80 : 0);
    setZeroFlag(result == 0);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(value & 0x01);
    return result;
}

void GameBoyEmulator::daa() {
    uint8_t a = registers.a;
    if (!getSubtractFlag()) {
        if (getCarryFlag() || a > 0x99) {
            a += 0x60;
            setCarryFlag(true);
        }
        if (getHalfCarryFlag() || (a & 0x0F) > 0x09) {
            a += 0x06;
        }
    } else {
        if (getCarryFlag()) {
            a -= 0x60;
        }
        if (getHalfCarryFlag()) {
            a -= 0x06;
        }
    }
    setZeroFlag(a == 0);
    setHalfCarryFlag(false);
    registers.a = a;
}

// Memory Management
uint8_t GameBoyEmulator::readMemory(uint16_t address) const {
    if (address < 0x4000) {
        return romBank0[address];
    } else if (address < 0x8000) {
        return romBankN[address - 0x4000];
    } else if (address < 0xA000) {
        return vram[address - 0x8000];
    } else if (address < 0xC000) {
        return externalRam[address - 0xA000];
    } else if (address < 0xD000) {
        return wramBank0[address - 0xC000];
    } else if (address < 0xE000) {
        return wramBankN[address - 0xD000];
    } else if (address < 0xFE00) {
        return readMemory(address - 0x2000); // Echo RAM
    } else if (address < 0xFEA0) {
        return oam[address - 0xFE00];
    } else if (address < 0xFF00) {
        return 0; // Unused
    } else if (address < 0xFF80) {
        return io[address - 0xFF00];
    } else if (address < 0xFFFF) {
        return hram[address - 0xFF80];
    } else {
        return interrupts.enable;
    }
}

void GameBoyEmulator::writeMemory(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        // ROM is read-only
        return;
    } else if (address < 0xA000) {
        vram[address - 0x8000] = value;
        updateTileData();
    } else if (address < 0xC000) {
        if (ramEnabled) {
            externalRam[address - 0xA000] = value;
        }
    } else if (address < 0xD000) {
        wramBank0[address - 0xC000] = value;
    } else if (address < 0xE000) {
        wramBankN[address - 0xD000] = value;
    } else if (address < 0xFE00) {
        writeMemory(address - 0x2000, value); // Echo RAM
    } else if (address < 0xFEA0) {
        oam[address - 0xFE00] = value;
        updateOAM();
    } else if (address < 0xFF00) {
        // Unused
        return;
    } else if (address < 0xFF80) {
        writeIO(address - 0xFF00, value);
    } else if (address < 0xFFFF) {
        hram[address - 0xFF80] = value;
    } else {
        interrupts.enable = value;
    }
}

void GameBoyEmulator::writeIO(uint8_t address, uint8_t value) {
    switch (address) {
        case 0x00: // P1/JOYP
            io[address] = value;
            updateJoypad();
            break;
        case 0x01: // SB
            io[address] = value;
            updateSerial();
            break;
        case 0x02: // SC
            io[address] = value;
            updateSerial();
            break;
        case 0x04: // DIV
            io[address] = 0;
            break;
        case 0x05: // TIMA
            io[address] = value;
            break;
        case 0x06: // TMA
            io[address] = value;
            break;
        case 0x07: // TAC
            io[address] = value;
            updateTimerControl();
            break;
        case 0x0F: // IF
            io[address] = value;
            updateInterruptFlags();
            break;
        case 0x40: // LCDC
            io[address] = value;
            updateLCDControl();
            break;
        case 0x41: // STAT
            io[address] = (io[address] & 0x07) | (value & 0xF8);
            updateLCDStatus();
            break;
        case 0x42: // SCY
            io[address] = value;
            updateScroll();
            break;
        case 0x43: // SCX
            io[address] = value;
            updateScroll();
            break;
        case 0x44: // LY
            io[address] = 0;
            break;
        case 0x45: // LYC
            io[address] = value;
            updateLCDStatus();
            break;
        case 0x46: // DMA
            io[address] = value;
            processDMA();
            break;
        case 0x47: // BGP
            io[address] = value;
            updatePalettes();
            break;
        case 0x48: // OBP0
            io[address] = value;
            updatePalettes();
            break;
        case 0x49: // OBP1
            io[address] = value;
            updatePalettes();
            break;
        case 0x4A: // WY
            io[address] = value;
            updateWindowPosition();
            break;
        case 0x4B: // WX
            io[address] = value;
            updateWindowPosition();
            break;
        default:
            io[address] = value;
            break;
    }
}

// PPU Functions
void GameBoyEmulator::updateLCDControl() {
    ppuEnabled = io[0x40] & 0x80;
    ppuWindowEnabled = io[0x40] & 0x20;
    ppuSpritesEnabled = io[0x40] & 0x02;
    ppuBackgroundEnabled = io[0x40] & 0x01;
}

void GameBoyEmulator::updateLCDStatus() {
    if (ppuEnabled) {
        if (graphics.ly == graphics.lyc) {
            graphics.stat |= 0x04;
            if (graphics.stat & 0x40) {
                interrupts.flags |= 0x02;
            }
        } else {
            graphics.stat &= ~0x04;
        }
    }
}

void GameBoyEmulator::updateScroll() {
    graphics.scx = io[0x43];
    graphics.scy = io[0x42];
}

void GameBoyEmulator::updateWindowPosition() {
    graphics.wx = io[0x4B];
    graphics.wy = io[0x4A];
}

void GameBoyEmulator::updatePalettes() {
    graphics.bgp = io[0x47];
    graphics.obp0 = io[0x48];
    graphics.obp1 = io[0x49];
}

void GameBoyEmulator::updateTileData() {
    // TODO: Implement tile data update
}

void GameBoyEmulator::updateOAM() {
    // TODO: Implement OAM update
}

void GameBoyEmulator::processDMA() {
    uint16_t source = io[0x46] << 8;
    dma.active = true;
    dma.source = source;
    dma.destination = 0xFE00;
    dma.length = 0xA0;
    dma.remaining = 0xA0;
}

// Timer Functions
void GameBoyEmulator::updateTimerControl() {
    timerEnabled = io[0x07] & 0x04;
    timerClock = io[0x07] & 0x03;
}

void GameBoyEmulator::updateInterruptFlags() {
    interrupts.flags = io[0x0F] & 0x1F;
}

void GameBoyEmulator::updateJoypad() {
    uint8_t joypad = io[0x00];
    if (!(joypad & 0x10)) {
        joypad = (joypad & 0xF0) | (input.buttons & 0x0F);
    } else if (!(joypad & 0x20)) {
        joypad = (joypad & 0xF0) | (input.directions & 0x0F);
    }
    io[0x00] = joypad;
}

void GameBoyEmulator::updateSerial() {
    // TODO: Implement serial transfer
}

void GameBoyEmulator::renderScanline() {
    if (!ppuEnabled) return;

    if (ppuMode == PPUMode::OAM_SCAN) {
        // Find sprites for this scanline
        findSpritesForScanline();
    } else if (ppuMode == PPUMode::PIXEL_TRANSFER) {
        // Render background
        if (ppuBackgroundEnabled) {
            renderBackground();
        }
        // Render window
        if (ppuWindowEnabled) {
            renderWindow();
        }
        // Render sprites
        if (ppuSpritesEnabled) {
            renderSprites();
        }
    }
}

void GameBoyEmulator::findSpritesForScanline() {
    spriteCount = 0;
    for (int i = 0; i < 40 && spriteCount < 10; i++) {
        uint16_t spriteAddr = 0xFE00 + (i * 4);
        int y = readMemory(spriteAddr) - 16;
        if (y <= graphics.ly && y + 8 > graphics.ly) {
            sprites[spriteCount].y = y;
            sprites[spriteCount].x = readMemory(spriteAddr + 1) - 8;
            sprites[spriteCount].tile = readMemory(spriteAddr + 2);
            sprites[spriteCount].attributes = readMemory(spriteAddr + 3);
            spriteCount++;
        }
    }
}

void GameBoyEmulator::renderBackground() {
    uint8_t scrollY = graphics.scy;
    uint8_t scrollX = graphics.scx;
    uint16_t tileMapAddr = (io[0x40] & 0x08) ? 0x9C00 : 0x9800;
    uint16_t tileDataAddr = (io[0x40] & 0x10) ? 0x8000 : 0x8800;

    for (int x = 0; x < 160; x++) {
        int tileX = (scrollX + x) / 8;
        int tileY = (scrollY + graphics.ly) / 8;
        uint16_t tileAddr = tileMapAddr + (tileY * 32) + tileX;
        uint8_t tileNum = readMemory(tileAddr);
        uint16_t tileOffset = tileDataAddr + (tileNum * 16);
        int pixelX = (scrollX + x) % 8;
        int pixelY = (scrollY + graphics.ly) % 8;
        uint8_t pixel = getTilePixel(tileOffset, pixelX, pixelY);
        setPixel(x, graphics.ly, pixel);
    }
}

void GameBoyEmulator::renderWindow() {
    if (graphics.wx > 166 || graphics.wy > 143) return;

    uint16_t tileMapAddr = (io[0x40] & 0x40) ? 0x9C00 : 0x9800;
    uint16_t tileDataAddr = (io[0x40] & 0x10) ? 0x8000 : 0x8800;

    for (int x = 0; x < 160; x++) {
        if (x + 7 < graphics.wx) continue;
        int tileX = (x - (graphics.wx - 7)) / 8;
        int tileY = graphics.ly / 8;
        uint16_t tileAddr = tileMapAddr + (tileY * 32) + tileX;
        uint8_t tileNum = readMemory(tileAddr);
        uint16_t tileOffset = tileDataAddr + (tileNum * 16);
        int pixelX = (x - (graphics.wx - 7)) % 8;
        int pixelY = graphics.ly % 8;
        uint8_t pixel = getTilePixel(tileOffset, pixelX, pixelY);
        setPixel(x, graphics.ly, pixel);
    }
}

void GameBoyEmulator::renderSprites() {
    for (int i = 0; i < spriteCount; i++) {
        Sprite& sprite = sprites[i];
        uint16_t tileOffset = 0x8000 + (sprite.tile * 16);
        bool flipX = sprite.attributes & 0x20;
        bool flipY = sprite.attributes & 0x40;
        bool priority = sprite.attributes & 0x80;
        uint8_t palette = (sprite.attributes & 0x10) ? graphics.obp1 : graphics.obp0;

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int pixelX = sprite.x + (flipX ? 7 - x : x);
                int pixelY = graphics.ly - sprite.y + (flipY ? 7 - y : y);
                if (pixelX >= 0 && pixelX < 160 && pixelY >= 0 && pixelY < 144) {
                    uint8_t pixel = getTilePixel(tileOffset, flipX ? 7 - x : x, flipY ? 7 - y : y);
                    if (pixel != 0) {
                        if (!priority || getPixel(pixelX, pixelY) == 0) {
                            setPixel(pixelX, pixelY, getPaletteColor(palette, pixel));
                        }
                    }
                }
            }
        }
    }
}

uint8_t GameBoyEmulator::getTilePixel(uint16_t tileAddr, int x, int y) {
    uint8_t byte1 = readMemory(tileAddr + (y * 2));
    uint8_t byte2 = readMemory(tileAddr + (y * 2) + 1);
    uint8_t bit1 = (byte1 >> (7 - x)) & 1;
    uint8_t bit2 = (byte2 >> (7 - x)) & 1;
    return (bit2 << 1) | bit1;
}

uint8_t GameBoyEmulator::getPaletteColor(uint8_t palette, uint8_t color) {
    return (palette >> (color * 2)) & 0x03;
}

void GameBoyEmulator::setPixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 160 && y >= 0 && y < 144) {
        frameBuffer[y * 160 + x] = color;
    }
}

uint8_t GameBoyEmulator::getPixel(int x, int y) const {
    if (x >= 0 && x < 160 && y >= 0 && y < 144) {
        return frameBuffer[y * 160 + x];
    }
    return 0;
}

// Enhanced PPU Functions
void GameBoyEmulator::updatePPU() {
    if (!ppuEnabled) {
        ppuMode = PPUMode::HBLANK;
        return;
    }

    // Update PPU mode
    switch (ppuMode) {
        case PPUMode::OAM_SCAN:
            if (ppuCycles >= 80) {
                ppuMode = PPUMode::PIXEL_TRANSFER;
                ppuCycles = 0;
            }
            break;
        case PPUMode::PIXEL_TRANSFER:
            if (ppuCycles >= 172) {
                ppuMode = PPUMode::HBLANK;
                ppuCycles = 0;
                renderScanline();
            }
            break;
        case PPUMode::HBLANK:
            if (ppuCycles >= 204) {
                ppuCycles = 0;
                graphics.ly++;
                if (graphics.ly == 144) {
                    ppuMode = PPUMode::VBLANK;
                    interrupts.flags |= 0x01; // VBlank interrupt
                } else {
                    ppuMode = PPUMode::OAM_SCAN;
                }
            }
            break;
        case PPUMode::VBLANK:
            if (ppuCycles >= 456) {
                ppuCycles = 0;
                graphics.ly++;
                if (graphics.ly > 153) {
                    graphics.ly = 0;
                    ppuMode = PPUMode::OAM_SCAN;
                }
            }
            break;
    }

    // Update PPU status
    updateLCDStatus();
    ppuCycles++;
}

void GameBoyEmulator::renderScanline() {
    if (!ppuEnabled) return;

    if (ppuMode == PPUMode::OAM_SCAN) {
        // Find sprites for this scanline
        findSpritesForScanline();
    } else if (ppuMode == PPUMode::PIXEL_TRANSFER) {
        // Render background
        if (ppuBackgroundEnabled) {
            renderBackground();
        }
        // Render window
        if (ppuWindowEnabled) {
            renderWindow();
        }
        // Render sprites
        if (ppuSpritesEnabled) {
            renderSprites();
        }
    }
}

void GameBoyEmulator::findSpritesForScanline() {
    spriteCount = 0;
    for (int i = 0; i < 40 && spriteCount < 10; i++) {
        uint16_t spriteAddr = 0xFE00 + (i * 4);
        int y = readMemory(spriteAddr) - 16;
        if (y <= graphics.ly && y + 8 > graphics.ly) {
            sprites[spriteCount].y = y;
            sprites[spriteCount].x = readMemory(spriteAddr + 1) - 8;
            sprites[spriteCount].tile = readMemory(spriteAddr + 2);
            sprites[spriteCount].attributes = readMemory(spriteAddr + 3);
            spriteCount++;
        }
    }
}

void GameBoyEmulator::renderBackground() {
    uint8_t scrollY = graphics.scy;
    uint8_t scrollX = graphics.scx;
    uint16_t tileMapAddr = (io[0x40] & 0x08) ? 0x9C00 : 0x9800;
    uint16_t tileDataAddr = (io[0x40] & 0x10) ? 0x8000 : 0x8800;

    for (int x = 0; x < 160; x++) {
        int tileX = (scrollX + x) / 8;
        int tileY = (scrollY + graphics.ly) / 8;
        uint16_t tileAddr = tileMapAddr + (tileY * 32) + tileX;
        uint8_t tileNum = readMemory(tileAddr);
        uint16_t tileOffset = tileDataAddr + (tileNum * 16);
        int pixelX = (scrollX + x) % 8;
        int pixelY = (scrollY + graphics.ly) % 8;
        uint8_t pixel = getTilePixel(tileOffset, pixelX, pixelY);
        setPixel(x, graphics.ly, pixel);
    }
}

void GameBoyEmulator::renderWindow() {
    if (graphics.wx > 166 || graphics.wy > 143) return;

    uint16_t tileMapAddr = (io[0x40] & 0x40) ? 0x9C00 : 0x9800;
    uint16_t tileDataAddr = (io[0x40] & 0x10) ? 0x8000 : 0x8800;

    for (int x = 0; x < 160; x++) {
        if (x + 7 < graphics.wx) continue;
        int tileX = (x - (graphics.wx - 7)) / 8;
        int tileY = graphics.ly / 8;
        uint16_t tileAddr = tileMapAddr + (tileY * 32) + tileX;
        uint8_t tileNum = readMemory(tileAddr);
        uint16_t tileOffset = tileDataAddr + (tileNum * 16);
        int pixelX = (x - (graphics.wx - 7)) % 8;
        int pixelY = graphics.ly % 8;
        uint8_t pixel = getTilePixel(tileOffset, pixelX, pixelY);
        setPixel(x, graphics.ly, pixel);
    }
}

void GameBoyEmulator::renderSprites() {
    for (int i = 0; i < spriteCount; i++) {
        Sprite& sprite = sprites[i];
        uint16_t tileOffset = 0x8000 + (sprite.tile * 16);
        bool flipX = sprite.attributes & 0x20;
        bool flipY = sprite.attributes & 0x40;
        bool priority = sprite.attributes & 0x80;
        uint8_t palette = (sprite.attributes & 0x10) ? graphics.obp1 : graphics.obp0;

        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int pixelX = sprite.x + (flipX ? 7 - x : x);
                int pixelY = graphics.ly - sprite.y + (flipY ? 7 - y : y);
                if (pixelX >= 0 && pixelX < 160 && pixelY >= 0 && pixelY < 144) {
                    uint8_t pixel = getTilePixel(tileOffset, flipX ? 7 - x : x, flipY ? 7 - y : y);
                    if (pixel != 0) {
                        if (!priority || getPixel(pixelX, pixelY) == 0) {
                            setPixel(pixelX, pixelY, getPaletteColor(palette, pixel));
                        }
                    }
                }
            }
        }
    }
}

uint8_t GameBoyEmulator::getTilePixel(uint16_t tileAddr, int x, int y) {
    uint8_t byte1 = readMemory(tileAddr + (y * 2));
    uint8_t byte2 = readMemory(tileAddr + (y * 2) + 1);
    uint8_t bit1 = (byte1 >> (7 - x)) & 1;
    uint8_t bit2 = (byte2 >> (7 - x)) & 1;
    return (bit2 << 1) | bit1;
}

uint8_t GameBoyEmulator::getPaletteColor(uint8_t palette, uint8_t color) {
    return (palette >> (color * 2)) & 0x03;
}

void GameBoyEmulator::setPixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 160 && y >= 0 && y < 144) {
        frameBuffer[y * 160 + x] = color;
    }
}

uint8_t GameBoyEmulator::getPixel(int x, int y) const {
    if (x >= 0 && x < 160 && y >= 0 && y < 144) {
        return frameBuffer[y * 160 + x];
    }
    return 0;
} 
