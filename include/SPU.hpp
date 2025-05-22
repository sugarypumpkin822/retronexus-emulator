#pragma once
#include <array>
#include <vector>
#include <cstdint>

// Sound Processing Unit for PlayStation systems
class SPU {
public:
    SPU(bool isPS2 = false);
    ~SPU() = default;

    // Core SPU functions
    void initialize();
    void reset();
    void step();

    // Memory access
    uint16_t read(uint32_t address) const;
    void write(uint32_t address, uint16_t value);

    // Audio processing
    void processVoice(uint8_t voice);
    void mixOutput();
    
    // State management
    bool saveState(const std::string& filepath);
    bool loadState(const std::string& filepath);

    // Audio buffer access
    const std::vector<int16_t>& getAudioBuffer() const { return audioBuffer; }
    void clearAudioBuffer() { audioBuffer.clear(); }

private:
    struct Voice {
        uint16_t volume;         // Voice volume
        uint16_t pitch;         // Voice pitch
        uint32_t startAddr;     // Sample start address
        uint32_t currentAddr;   // Current playing address
        uint16_t adsr1, adsr2;  // ADSR envelope values
        uint16_t adsrVolume;    // Current ADSR volume
        bool keyOn;             // Key on flag
        bool keyOff;            // Key off flag
    };

    static constexpr size_t PS1_VOICE_COUNT = 24;
    static constexpr size_t PS2_VOICE_COUNT = 48;
    static constexpr size_t SPU_RAM_SIZE = 512 * 1024;  // 512KB for PS1, more for PS2

    std::vector<Voice> voices;
    std::vector<uint8_t> spuRam;
    std::vector<int16_t> audioBuffer;

    uint16_t mainVolume;
    uint16_t reverbVolume;
    uint32_t transferAddress;
    bool reverbEnabled;
    bool irqEnabled;
    bool transferMode;
    bool isPS2Mode;

    // Internal helper functions
    void updateADSR(Voice& voice);
    void processReverb();
    void checkIRQ();
}; 
