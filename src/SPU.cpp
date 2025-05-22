#include "../include/SPU.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>

SPU::SPU(bool isPS2) : isPS2Mode(isPS2) {
    // Initialize vectors with appropriate sizes
    voices.resize(isPS2 ? PS2_VOICE_COUNT : PS1_VOICE_COUNT);
    spuRam.resize(isPS2 ? SPU_RAM_SIZE * 2 : SPU_RAM_SIZE);  // PS2 has double the SPU RAM
    audioBuffer.reserve(44100 * 2);  // Reserve space for 1 second of stereo audio at 44.1kHz

    initialize();
}

void SPU::initialize() {
    mainVolume = 0x3FFF;  // Maximum volume
    reverbVolume = 0;
    transferAddress = 0;
    reverbEnabled = false;
    irqEnabled = false;
    transferMode = false;

    // Initialize all voices
    for (auto& voice : voices) {
        voice.volume = 0;
        voice.pitch = 0;
        voice.startAddr = 0;
        voice.currentAddr = 0;
        voice.adsr1 = 0;
        voice.adsr2 = 0;
        voice.adsrVolume = 0;
        voice.keyOn = false;
        voice.keyOff = false;
    }

    // Clear SPU RAM
    std::fill(spuRam.begin(), spuRam.end(), 0);
}

void SPU::reset() {
    initialize();
    audioBuffer.clear();
}

void SPU::step() {
    // Process each active voice
    for (uint8_t i = 0; i < voices.size(); ++i) {
        if (voices[i].keyOn) {
            processVoice(i);
        }
    }

    // Mix all voices and apply reverb if enabled
    mixOutput();
    
    if (reverbEnabled) {
        processReverb();
    }

    checkIRQ();
}

void SPU::processVoice(uint8_t voiceIndex) {
    if (voiceIndex >= voices.size()) return;

    auto& voice = voices[voiceIndex];
    
    // Update ADSR envelope
    updateADSR(voice);

    // Read sample data from SPU RAM
    uint32_t addr = voice.currentAddr & (spuRam.size() - 1);
    int16_t sample = static_cast<int16_t>((spuRam[addr + 1] << 8) | spuRam[addr]);

    // Apply ADSR volume
    sample = static_cast<int16_t>((static_cast<int32_t>(sample) * voice.adsrVolume) >> 15);

    // Apply voice volume
    sample = static_cast<int16_t>((static_cast<int32_t>(sample) * voice.volume) >> 15);

    // Add to audio buffer
    audioBuffer.push_back(sample);

    // Update current address based on pitch
    voice.currentAddr += (voice.pitch >> 8) * 2;  // 16-bit samples
}

void SPU::mixOutput() {
    if (audioBuffer.empty()) return;

    // Apply main volume to all samples
    for (auto& sample : audioBuffer) {
        sample = static_cast<int16_t>((static_cast<int32_t>(sample) * mainVolume) >> 15);
    }
}

void SPU::updateADSR(Voice& voice) {
    // Simple ADSR implementation
    if (voice.keyOn) {
        // Attack phase
        voice.adsrVolume = std::min(voice.adsrVolume + (voice.adsr1 >> 8), 0x7FFFu);
    } else if (voice.keyOff) {
        // Release phase
        voice.adsrVolume = std::max(voice.adsrVolume - (voice.adsr2 & 0xFF), 0u);
    }
}

void SPU::processReverb() {
    // Basic reverb implementation
    if (!reverbEnabled || audioBuffer.empty()) return;

    std::vector<int16_t> reverbBuffer = audioBuffer;
    for (size_t i = 0; i < audioBuffer.size(); ++i) {
        // Simple delay and attenuation
        if (i >= 2048) {  // Delay of 2048 samples
            audioBuffer[i] += static_cast<int16_t>((static_cast<int32_t>(reverbBuffer[i - 2048]) * reverbVolume) >> 15);
        }
    }
}

void SPU::checkIRQ() {
    if (!irqEnabled) return;

    // Check if current address matches IRQ address
    // Implementation depends on specific PlayStation model
}

uint16_t SPU::read(uint32_t address) const {
    address &= (spuRam.size() - 1);
    if (address + 1 >= spuRam.size()) return 0;
    return (spuRam[address + 1] << 8) | spuRam[address];
}

void SPU::write(uint32_t address, uint16_t value) {
    address &= (spuRam.size() - 1);
    if (address + 1 >= spuRam.size()) return;
    spuRam[address] = value & 0xFF;
    spuRam[address + 1] = (value >> 8) & 0xFF;
}

bool SPU::saveState(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) return false;

    // Save SPU state
    file.write(reinterpret_cast<const char*>(&mainVolume), sizeof(mainVolume));
    file.write(reinterpret_cast<const char*>(&reverbVolume), sizeof(reverbVolume));
    file.write(reinterpret_cast<const char*>(&transferAddress), sizeof(transferAddress));
    file.write(reinterpret_cast<const char*>(&reverbEnabled), sizeof(reverbEnabled));
    file.write(reinterpret_cast<const char*>(&irqEnabled), sizeof(irqEnabled));
    file.write(reinterpret_cast<const char*>(&transferMode), sizeof(transferMode));

    // Save voice states
    for (const auto& voice : voices) {
        file.write(reinterpret_cast<const char*>(&voice), sizeof(Voice));
    }

    // Save SPU RAM
    file.write(reinterpret_cast<const char*>(spuRam.data()), spuRam.size());

    return true;
}

bool SPU::loadState(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return false;

    // Load SPU state
    file.read(reinterpret_cast<char*>(&mainVolume), sizeof(mainVolume));
    file.read(reinterpret_cast<char*>(&reverbVolume), sizeof(reverbVolume));
    file.read(reinterpret_cast<char*>(&transferAddress), sizeof(transferAddress));
    file.read(reinterpret_cast<char*>(&reverbEnabled), sizeof(reverbEnabled));
    file.read(reinterpret_cast<char*>(&irqEnabled), sizeof(irqEnabled));
    file.read(reinterpret_cast<char*>(&transferMode), sizeof(transferMode));

    // Load voice states
    for (auto& voice : voices) {
        file.read(reinterpret_cast<char*>(&voice), sizeof(Voice));
    }

    // Load SPU RAM
    file.read(reinterpret_cast<char*>(spuRam.data()), spuRam.size());

    return true;
} 
