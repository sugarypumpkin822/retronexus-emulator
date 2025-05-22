#pragma once
#include "ConsoleEmulator.hpp"
#include <array>
#include <bitset>
#include <unordered_map>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>

// GameBoy-specific constants
constexpr uint16_t ROM_BANK_SIZE = 0x4000;
constexpr uint16_t RAM_BANK_SIZE = 0x2000;
constexpr uint16_t VRAM_SIZE = 0x2000;
constexpr uint16_t OAM_SIZE = 0xA0;
constexpr uint16_t IO_SIZE = 0x80;
constexpr uint16_t HRAM_SIZE = 0x7F;

// GameBoy Color specific constants
constexpr uint8_t GBC_PALETTE_COUNT = 8;
constexpr uint8_t GBC_SPRITE_PALETTE_COUNT = 8;
constexpr uint8_t GBC_DMA_TRANSFER_SIZE = 0xA0;

// GameBoy memory map
enum class MemoryRegion {
    ROM_BANK_0,
    ROM_BANK_N,
    VRAM,
    EXTERNAL_RAM,
    WRAM_BANK_0,
    WRAM_BANK_N,
    ECHO_RAM,
    OAM,
    UNUSED,
    IO,
    HRAM,
    INTERRUPT_ENABLE
};

// GameBoy PPU modes
enum class PPUMode {
    HBLANK,
    VBLANK,
    OAM_SCAN,
    PIXEL_TRANSFER
};

// GameBoy interrupt types
enum class InterruptType {
    VBLANK,
    LCD_STAT,
    TIMER,
    SERIAL,
    JOYPAD
};

class GameBoyEmulator : public ConsoleEmulator {
public:
    GameBoyEmulator();
    ~GameBoyEmulator() override = default;

    // Core emulation functions
    bool initialize() override;
    void step() override;
    void reset() override;
    bool loadROM(const std::string& romPath) override;

    // Memory management
    uint8_t readMemory(uint16_t address) const override;
    void writeMemory(uint16_t address, uint8_t value) override;

    // State management
    bool saveState(const std::string& filename = "") override;
    bool loadState(const std::string& filename = "") override;
    bool hasSaveState() const override;

    // Console specific information
    ConsoleType getConsoleType() const override { return ConsoleType::GAMEBOY; }
    std::string getConsoleName() const override { return "Nintendo Game Boy"; }
    
    // System requirements
    uint32_t getMinimumMemorySize() const override { return 32 * 1024; } // 32KB
    uint32_t getRecommendedMemorySize() const override { return 64 * 1024; } // 64KB

    // Configuration
    void setFrameRate(int fps) override;
    void setAudioEnabled(bool enabled) override;
    void setVideoEnabled(bool enabled) override;
    void setInputEnabled(bool enabled) override;
    void setDebugMode(bool enabled) override;

    // Debug functions
    void stepFrame() override;
    void breakpoint() override;
    void continueExecution() override;
    void setBreakpoint(uint16_t address) override;
    void clearBreakpoint(uint16_t address) override;
    void clearAllBreakpoints() override;

    // Memory access
    void dumpMemory(uint16_t start, uint16_t end) const override;

    // Register access
    uint16_t getPC() const override;
    uint16_t getSP() const override;
    uint8_t getA() const override;
    uint8_t getB() const override;
    uint8_t getC() const override;
    uint8_t getD() const override;
    uint8_t getE() const override;
    uint8_t getH() const override;
    uint8_t getL() const override;
    uint16_t getAF() const override;
    uint16_t getBC() const override;
    uint16_t getDE() const override;
    uint16_t getHL() const override;

    // Status flags
    bool getZeroFlag() const override;
    bool getSubtractFlag() const override;
    bool getHalfCarryFlag() const override;
    bool getCarryFlag() const override;

    // System information
    std::string getSystemName() const override;
    std::string getROMName() const override;
    std::string getROMSize() const override;
    std::string getROMMapper() const override;
    std::string getROMSaveType() const override;
    std::string getROMRegion() const override;

    // Performance monitoring
    double getFPS() const override;
    double getFrameTime() const override;
    double getCPUTime() const override;
    double getPPUTime() const override;
    double getAPUTime() const override;

    // Event callbacks
    void setFrameCallback(FrameCallback callback) override;
    void setAudioCallback(AudioCallback callback) override;
    void setInputCallback(InputCallback callback) override;
    void setDebugCallback(DebugCallback callback) override;

    // GameBoy-specific features
    void setColorMode(bool enabled);
    bool isColorMode() const;
    void setDoubleSpeed(bool enabled);
    bool isDoubleSpeed() const;
    void setInfrared(bool enabled);
    bool isInfrared() const;
    void setRumble(bool enabled);
    bool isRumble() const;

    // Memory management
    void setBreakpoint(uint16_t address, std::function<void()> callback);
    void clearBreakpoint(uint16_t address);
    void setWatchpoint(uint16_t address, std::function<void(uint8_t)> callback);
    void clearWatchpoint(uint16_t address);

    // Debugging tools
    void stepInstruction();
    void setTraceLogging(bool enabled);
    bool isTraceLogging() const;
    std::string getDisassembly(uint16_t address) const;
    std::vector<std::string> getTraceLog() const;
    void clearTraceLog();

    // Graphics debugging
    void setTileViewer(bool enabled);
    bool isTileViewer() const;
    void setSpriteViewer(bool enabled);
    bool isSpriteViewer() const;
    void setPaletteViewer(bool enabled);
    bool isPaletteViewer() const;
    void setVRAMViewer(bool enabled);
    bool isVRAMViewer() const;

    // Audio debugging
    void setAudioChannelEnabled(uint8_t channel, bool enabled);
    bool isAudioChannelEnabled(uint8_t channel) const;
    void setAudioWaveform(uint8_t channel, const std::array<uint8_t, 32>& waveform);
    std::array<uint8_t, 32> getAudioWaveform(uint8_t channel) const;

    // Performance monitoring
    void setPerformanceCounter(bool enabled);
    bool isPerformanceCounter() const;
    uint64_t getInstructionCount() const;
    uint64_t getCycleCount() const;
    double getAverageCyclesPerFrame() const;

protected:
    bool validateROM(const std::vector<uint8_t>& data) const override;
    bool detectConsoleType(const std::vector<uint8_t>& data) const override;

private:
    // GameBoy-specific memory
    std::array<uint8_t, ROM_BANK_SIZE> romBank0;
    std::vector<uint8_t> romBankN;
    std::array<uint8_t, VRAM_SIZE> vram;
    std::vector<uint8_t> externalRam;
    std::array<uint8_t, RAM_BANK_SIZE> wramBank0;
    std::vector<uint8_t> wramBankN;
    std::array<uint8_t, OAM_SIZE> oam;
    std::array<uint8_t, IO_SIZE> io;
    std::array<uint8_t, HRAM_SIZE> hram;

    // GameBoy-specific state
    uint8_t currentRomBank;
    uint8_t currentRamBank;
    bool ramEnabled;
    bool batteryBacked;
    std::string romPath;
    std::string savePath;

    // PPU state
    PPUMode ppuMode;
    uint8_t ppuModeClock;
    uint8_t ppuLine;
    uint8_t ppuScrollX;
    uint8_t ppuScrollY;
    uint8_t ppuWindowX;
    uint8_t ppuWindowY;
    bool ppuWindowEnabled;
    bool ppuEnabled;
    bool ppuBackgroundEnabled;
    bool ppuSpritesEnabled;
    uint8_t ppuBackgroundPalette;
    std::array<uint8_t, 8> ppuSpritePalettes;

    // Timer state
    uint16_t timerDivider;
    uint8_t timerCounter;
    uint8_t timerModulo;
    bool timerEnabled;
    uint8_t timerClock;

    // Interrupt state
    std::bitset<5> interruptFlags;
    std::bitset<5> interruptEnable;
    bool interruptsEnabled;

    // Debug state
    std::unordered_map<uint16_t, bool> breakpoints;
    bool debugStepping;
    bool debugPaused;

    // Feature flags
    bool colorMode;
    bool doubleSpeed;
    bool infrared;
    bool rumble;
    bool traceLogging;
    bool tileViewer;
    bool spriteViewer;
    bool paletteViewer;
    bool vramViewer;
    bool performanceCounter;

    // Debugging state
    std::map<uint16_t, std::function<void()>> breakpoints;
    std::map<uint16_t, std::function<void(uint8_t)>> watchpoints;
    std::vector<std::string> traceLog;
    uint64_t instructionCount;
    uint64_t cycleCount;
    uint64_t frameCount;

    // Audio state
    std::array<bool, 4> audioChannels;
    std::array<std::array<uint8_t, 32>, 4> audioWaveforms;

    // Helper functions
    void updatePPU(int cycles);
    void updateTimer(int cycles);
    void handleInterrupts();
    void processDMA();
    void updateJoypad();
    void updateSerial();
    void updateAudio();
    void renderScanline();
    void renderBackground();
    void renderSprites();
    void updateTileData();
    void updateTileMaps();
    void updateOAM();
    void updatePalettes();
    void updateWindow();
    void updateSprites();
    void updateBackground();
    void updateScroll();
    void updateWindowPosition();
    void updateLCDStatus();
    void updateInterrupts();
    void updateTimers();
    void updateSerial();
    void updateAudio();
    void updateInput();
    void updateDMA();
    void updateHDMA();
    void updateVRAM();
    void updateOAM();
    void updateIO();
    void updateHRAM();
    void updateInterruptEnable();
    void updateInterruptFlags();
    void updateTimerControl();
    void updateTimerModulo();
    void updateTimerCounter();
    void updateDivider();
    void updateSerialControl();
    void updateSerialData();
    void updateAudioControl();
    void updateAudioChannel1();
    void updateAudioChannel2();
    void updateAudioChannel3();
    void updateAudioChannel4();
    void updateAudioOutput();
    void updateAudioVolume();
    void updateAudioPanning();
    void updateAudioSampleRate();
    void updateAudioBuffer();
    void updateAudioCallback();
    void updateFrameCallback();
    void updateInputCallback();
    void updateDebugCallback();
    void updatePerformance();
    void updateState();
    void updateSaveState();
    void updateLoadState();
    void updateBreakpoints();
    void updateDebugMode();
    void updatePause();
    void updateResume();
    void updateStop();
    void updateReset();
    void updateInitialize();
    void updateLoadROM();
    void updateRunFrame();
    void updateStep();
    void updateStepFrame();
    void updateBreakpoint();
    void updateContinueExecution();
    void updateSetBreakpoint();
    void updateClearBreakpoint();
    void updateClearAllBreakpoints();
    void updateReadMemory();
    void updateWriteMemory();
    void updateDumpMemory();
    void updateGetPC();
    void updateGetSP();
    void updateGetA();
    void updateGetB();
    void updateGetC();
    void updateGetD();
    void updateGetE();
    void updateGetH();
    void updateGetL();
    void updateGetAF();
    void updateGetBC();
    void updateGetDE();
    void updateGetHL();
    void updateGetZeroFlag();
    void updateGetSubtractFlag();
    void updateGetHalfCarryFlag();
    void updateGetCarryFlag();
    void updateGetSystemName();
    void updateGetROMName();
    void updateGetROMSize();
    void updateGetROMMapper();
    void updateGetROMSaveType();
    void updateGetROMRegion();
    void updateGetFPS();
    void updateGetFrameTime();
    void updateGetCPUTime();
    void updateGetPPUTime();
    void updateGetAPUTime();
    void updateSetFrameCallback();
    void updateSetAudioCallback();
    void updateSetInputCallback();
    void updateSetDebugCallback();
    void updatePerformanceCounters();
    void renderDebugViews();
    void renderTileViewer();
    void renderSpriteViewer();
    void renderPaletteViewer();
    void renderVRAMViewer();
    void renderPerformanceCounter();
}; 
