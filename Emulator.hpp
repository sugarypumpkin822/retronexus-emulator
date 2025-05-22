#pragma once
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <unordered_map>
#include "ConsoleType.hpp"
#include "ConsoleEmulator.hpp"
#include <functional>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <map>
#include <SDL2/SDL.h>

// Forward declarations
class AudioSystem;
class VideoSystem;
class InputSystem;
class MemorySystem;
class CPUSystem;
class PPUSystem;
class APUSystem;
class TimerSystem;
class SaveState;
class AudioConfig;
class InputMapping;
class PerformanceMonitor;

// Emulator state
enum class EmulatorState {
    STOPPED,
    RUNNING,
    PAUSED,
    STEPPING,
    DEBUGGING,
    REWINDING,
    SAVING_STATE,
    LOADING_STATE
};

// Debug event types
enum class DebugEventType {
    BREAKPOINT,
    WATCHPOINT,
    MEMORY_ACCESS,
    REGISTER_CHANGE,
    INTERRUPT,
    TIMER,
    DMA,
    AUDIO,
    GRAPHICS,
    INPUT
};

// Debug event structure
struct DebugEvent {
    DebugEventType type;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> details;
};

// Callback types
using InputCallback = std::function<void(const SDL_Event&)>;
using DebugCallback = std::function<void(const DebugEvent&)>;
using StateCallback = std::function<void(EmulatorState)>;
using ErrorCallback = std::function<void(const std::string&)>;

class Emulator {
public:
    Emulator();
    virtual ~Emulator();

    // Console management
    bool setConsoleType(ConsoleType type);
    ConsoleType getConsoleType() const;
    std::string getConsoleName() const;

    // File loading
    bool loadFile(const std::string& filepath);
    
    // Memory management
    void writeMemory(uint32_t address, uint8_t value);
    uint8_t readMemory(uint32_t address) const;
    
    // Emulation control
    virtual bool initialize() = 0;
    virtual void runFrame() = 0;
    virtual void reset() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;

    // State management
    virtual bool saveState(const std::string& filename = "") = 0;
    virtual bool loadState(const std::string& filename = "") = 0;
    virtual bool hasSaveState() const = 0;

    // Configuration
    virtual void setFrameRate(int fps) = 0;
    virtual void setAudioEnabled(bool enabled) = 0;
    virtual void setVideoEnabled(bool enabled) = 0;
    virtual void setInputEnabled(bool enabled) = 0;
    virtual void setDebugMode(bool enabled) = 0;

    // Debug functions
    virtual void step() = 0;
    virtual void stepFrame() = 0;
    virtual void breakpoint() = 0;
    virtual void continueExecution() = 0;
    virtual void setBreakpoint(uint16_t address) = 0;
    virtual void clearBreakpoint(uint16_t address) = 0;
    virtual void clearAllBreakpoints() = 0;

    // Memory access
    virtual uint8_t readMemory(uint16_t address) const = 0;
    virtual void writeMemory(uint16_t address, uint8_t value) = 0;
    virtual void dumpMemory(uint16_t start, uint16_t end) const = 0;

    // Register access
    virtual uint16_t getPC() const = 0;
    virtual uint16_t getSP() const = 0;
    virtual uint8_t getA() const = 0;
    virtual uint8_t getB() const = 0;
    virtual uint8_t getC() const = 0;
    virtual uint8_t getD() const = 0;
    virtual uint8_t getE() const = 0;
    virtual uint8_t getH() const = 0;
    virtual uint8_t getL() const = 0;
    virtual uint16_t getAF() const = 0;
    virtual uint16_t getBC() const = 0;
    virtual uint16_t getDE() const = 0;
    virtual uint16_t getHL() const = 0;

    // Status flags
    virtual bool getZeroFlag() const = 0;
    virtual bool getSubtractFlag() const = 0;
    virtual bool getHalfCarryFlag() const = 0;
    virtual bool getCarryFlag() const = 0;

    // System information
    virtual std::string getSystemName() const = 0;
    virtual std::string getROMName() const = 0;
    virtual std::string getROMSize() const = 0;
    virtual std::string getROMMapper() const = 0;
    virtual std::string getROMSaveType() const = 0;
    virtual std::string getROMRegion() const = 0;

    // Performance monitoring
    virtual double getFPS() const = 0;
    virtual double getFrameTime() const = 0;
    virtual double getCPUTime() const = 0;
    virtual double getPPUTime() const = 0;
    virtual double getAPUTime() const = 0;

    // Event callbacks
    using FrameCallback = std::function<void(const std::vector<uint8_t>&)>;
    using AudioCallback = std::function<void(const std::vector<float>&)>;
    virtual void setFrameCallback(FrameCallback callback) = 0;
    virtual void setAudioCallback(AudioCallback callback) = 0;

    // State management
    void setState(EmulatorState state);
    EmulatorState getState() const;
    void setStateCallback(StateCallback callback);
    void setErrorCallback(ErrorCallback callback);
    bool isRunning() const;
    bool isPaused() const;
    bool isDebugging() const;
    bool isRewinding() const;

    // Configuration
    void setAudioConfig(const AudioConfig& config);
    void setInputMapping(const InputMapping& mapping);
    void setCheatsEnabled(bool enabled);
    void setRewindEnabled(bool enabled);
    void setRewindBufferSize(size_t size);
    void setFrameLimit(bool enabled);
    void setVSync(bool enabled);
    void setFullscreen(bool enabled);
    void setBilinearFiltering(bool enabled);

    // Input handling
    void setInputCallback(InputCallback callback);
    void handleInput(const SDL_Event& event);
    void setKeyState(SDL_Scancode key, bool pressed);
    bool isKeyPressed(SDL_Scancode key) const;
    void setControllerState(int controller, int button, bool pressed);
    bool isControllerButtonPressed(int controller, int button) const;

    // Debugging
    void setDebugCallback(DebugCallback callback);
    void addBreakpoint(uint16_t address);
    void removeBreakpoint(uint16_t address);
    void addWatchpoint(uint16_t address);
    void removeWatchpoint(uint16_t address);
    void stepInstruction();
    void stepFrame();
    void toggleDebugger();
    void setDebugStepping(bool enabled);
    bool isDebugStepping() const;
    void setDebugPaused(bool enabled);
    bool isDebugPaused() const;

    // Performance monitoring
    void setPerformanceMonitor(std::shared_ptr<PerformanceMonitor> monitor);
    std::shared_ptr<PerformanceMonitor> getPerformanceMonitor() const;
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    bool isPerformanceMonitoring() const;

    // Rewind functionality
    void startRewind();
    void stopRewind();
    void setRewindSpeed(double speed);
    double getRewindSpeed() const;
    size_t getRewindBufferSize() const;
    size_t getCurrentRewindPosition() const;

    // Save state management
    void saveStateToFile(const std::string& path);
    void loadStateFromFile(const std::string& path);
    void setAutoSaveEnabled(bool enabled);
    bool isAutoSaveEnabled() const;
    void setAutoSaveInterval(size_t frames);
    size_t getAutoSaveInterval() const;

protected:
    // Console emulator instance
    std::unique_ptr<ConsoleEmulator> console;
    
    // Emulation state
    bool isRunning;
    std::vector<uint8_t> fileData;

    // System components
    std::unique_ptr<AudioSystem> audio;
    std::unique_ptr<VideoSystem> video;
    std::unique_ptr<InputSystem> input;
    std::unique_ptr<MemorySystem> memory;
    std::unique_ptr<CPUSystem> cpu;
    std::unique_ptr<PPUSystem> ppu;
    std::unique_ptr<APUSystem> apu;
    std::unique_ptr<TimerSystem> timer;

    // State management
    std::atomic<bool> running;
    std::atomic<bool> paused;
    std::atomic<bool> debugMode;
    std::atomic<bool> rewinding;
    std::atomic<bool> debugStepping;
    std::atomic<bool> debugPaused;
    std::mutex stateMutex;
    std::queue<SaveState> saveStates;

    // Performance monitoring
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    std::chrono::high_resolution_clock::time_point lastCPUTime;
    std::chrono::high_resolution_clock::time_point lastPPUTime;
    std::chrono::high_resolution_clock::time_point lastAPUTime;
    double frameTime;
    double cpuTime;
    double ppuTime;
    double apuTime;

    // Callbacks
    FrameCallback frameCallback;
    AudioCallback audioCallback;
    InputCallback inputCallback;
    DebugCallback debugCallback;
    StateCallback stateCallback;
    ErrorCallback errorCallback;

    // Configuration
    bool audioEnabled;
    AudioConfig audioConfig;
    InputMapping inputMapping;
    bool cheatsEnabled;
    bool rewindEnabled;
    size_t rewindBufferSize;
    bool frameLimit;
    bool vsync;
    bool fullscreen;
    bool bilinearFiltering;

    // Performance monitoring
    std::shared_ptr<PerformanceMonitor> performanceMonitor;
    bool performanceMonitoring;

    // Rewind state
    double rewindSpeed;
    size_t currentRewindPosition;
    std::queue<std::vector<uint8_t>> rewindBuffer;

    // Save state
    bool autoSaveEnabled;
    size_t autoSaveInterval;
    size_t framesSinceLastSave;

    // Helper functions
    ConsoleType detectConsoleType(const std::vector<uint8_t>& data) const;
    std::unique_ptr<ConsoleEmulator> createConsoleEmulator(ConsoleType type) const;
    void updateState(EmulatorState newState);
    void handleDebugEvent(const DebugEvent& event);
    void handleError(const std::string& error);
    void updateInputState();
    void updatePerformanceMonitoring();
    void updateRewindState();
    void updateAutoSave();
    void saveRewindState();
    void loadRewindState();
    void clearRewindBuffer();
    void updateSetInputCallback();
    void updateSetDebugCallback();
}; 
