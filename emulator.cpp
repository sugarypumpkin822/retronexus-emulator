#include "Emulator.hpp"
#include "GameBoyEmulator.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <SDL2/SDL.h>

Emulator::Emulator() : running(false), paused(false), debugMode(false), rewinding(false),
    debugStepping(false), debugPaused(false), audioEnabled(true), cheatsEnabled(false),
    rewindEnabled(false), rewindBufferSize(60), frameLimit(true), vsync(true),
    fullscreen(false), bilinearFiltering(false), performanceMonitoring(false),
    rewindSpeed(1.0), currentRewindPosition(0), autoSaveEnabled(false),
    autoSaveInterval(300), framesSinceLastSave(0) {
    initialize();
}

Emulator::~Emulator() {
    stop();
    cleanup();
}

bool Emulator::setConsoleType(ConsoleType type) {
    console = createConsoleEmulator(type);
    return console != nullptr;
}

ConsoleType Emulator::getConsoleType() const {
    return console ? console->getConsoleType() : ConsoleType::UNKNOWN;
}

std::string Emulator::getConsoleName() const {
    return console ? console->getConsoleName() : "Unknown";
}

bool Emulator::loadFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // Read the entire file into fileData
    fileData.clear();
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    fileData.resize(fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

    // Auto-detect console type if not set
    if (!console) {
        ConsoleType detectedType = detectConsoleType(fileData);
        if (!setConsoleType(detectedType)) {
            std::cerr << "Failed to create emulator for detected console type" << std::endl;
            return false;
        }
    }

    return console->loadROM(fileData);
}

void Emulator::writeMemory(uint32_t address, uint8_t value) {
    if (console) {
        console->writeMemory(address, value);
    }
}

uint8_t Emulator::readMemory(uint32_t address) const {
    if (console) {
        return console->readMemory(address);
    }
    throw std::runtime_error("No console emulator initialized");
}

void Emulator::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    // Initialize systems
    audio = std::make_unique<AudioSystem>();
    video = std::make_unique<VideoSystem>();
    input = std::make_unique<InputSystem>();
    memory = std::make_unique<MemorySystem>();
    cpu = std::make_unique<CPUSystem>();
    ppu = std::make_unique<PPUSystem>();
    apu = std::make_unique<APUSystem>();
    timer = std::make_unique<TimerSystem>();

    // Initialize performance monitoring
    if (performanceMonitor) {
        performanceMonitor->initialize();
    }

    // Initialize rewind buffer
    clearRewindBuffer();
}

void Emulator::cleanup() {
    // Clean up systems
    audio.reset();
    video.reset();
    input.reset();
    memory.reset();
    cpu.reset();
    ppu.reset();
    apu.reset();
    timer.reset();

    // Clean up SDL
    SDL_Quit();
}

void Emulator::runFrame() {
    if (!running || paused) return;

    auto frameStart = std::chrono::high_resolution_clock::now();

    // Update input state
    updateInputState();

    // Run CPU
    auto cpuStart = std::chrono::high_resolution_clock::now();
    cpu->run();
    auto cpuEnd = std::chrono::high_resolution_clock::now();
    cpuTime = std::chrono::duration<double>(cpuEnd - cpuStart).count();

    // Run PPU
    auto ppuStart = std::chrono::high_resolution_clock::now();
    ppu->run();
    auto ppuEnd = std::chrono::high_resolution_clock::now();
    ppuTime = std::chrono::duration<double>(ppuEnd - ppuStart).count();

    // Run APU
    auto apuStart = std::chrono::high_resolution_clock::now();
    apu->run();
    auto apuEnd = std::chrono::high_resolution_clock::now();
    apuTime = std::chrono::duration<double>(apuEnd - apuStart).count();

    // Update performance monitoring
    if (performanceMonitoring && performanceMonitor) {
        performanceMonitor->startFrame();
        performanceMonitor->startCPUMeasurement();
        performanceMonitor->startGPUMeasurement();
        performanceMonitor->endCPUMeasurement();
        performanceMonitor->endGPUMeasurement();
        performanceMonitor->endFrame();
    }

    // Handle rewind
    if (rewinding) {
        updateRewindState();
    } else if (rewindEnabled) {
        saveRewindState();
    }

    // Handle auto-save
    if (autoSaveEnabled) {
        updateAutoSave();
    }

    // Calculate frame time
    auto frameEnd = std::chrono::high_resolution_clock::now();
    frameTime = std::chrono::duration<double>(frameEnd - frameStart).count();

    // Frame limiting
    if (frameLimit) {
        double targetFrameTime = 1.0 / 60.0; // 60 FPS
        if (frameTime < targetFrameTime) {
            SDL_Delay(static_cast<uint32_t>((targetFrameTime - frameTime) * 1000.0));
        }
    }
}

void Emulator::updateInputState() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                stop();
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                handleKeyboardEvent(event.key);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                handleControllerEvent(event.cbutton);
                break;
            case SDL_CONTROLLERAXISMOTION:
                handleControllerAxisEvent(event.caxis);
                break;
        }

        if (inputCallback) {
            inputCallback(event);
        }
    }
}

void Emulator::handleKeyboardEvent(const SDL_KeyboardEvent& event) {
    bool pressed = event.type == SDL_KEYDOWN;
    SDL_Scancode scancode = event.keysym.scancode;

    // Update key state
    setKeyState(scancode, pressed);

    // Handle debug keys
    if (debugMode) {
        switch (scancode) {
            case SDL_SCANCODE_F1:
                if (pressed) toggleDebugger();
                break;
            case SDL_SCANCODE_F2:
                if (pressed) stepInstruction();
                break;
            case SDL_SCANCODE_F3:
                if (pressed) stepFrame();
                break;
            case SDL_SCANCODE_F4:
                if (pressed) setDebugPaused(!isDebugPaused());
                break;
            case SDL_SCANCODE_F5:
                if (pressed) saveStateToFile("quicksave.sav");
                break;
            case SDL_SCANCODE_F6:
                if (pressed) loadStateFromFile("quicksave.sav");
                break;
            case SDL_SCANCODE_F7:
                if (pressed) startRewind();
                break;
            case SDL_SCANCODE_F8:
                if (pressed) stopRewind();
                break;
        }
    }
}

void Emulator::handleControllerEvent(const SDL_ControllerButtonEvent& event) {
    bool pressed = event.type == SDL_CONTROLLERBUTTONDOWN;
    setControllerState(event.which, event.button, pressed);
}

void Emulator::handleControllerAxisEvent(const SDL_ControllerAxisEvent& event) {
    // Convert analog input to digital if needed
    bool pressed = std::abs(event.value) > 16384; // 50% of max value
    setControllerState(event.which, event.axis, pressed);
}

void Emulator::updateRewindState() {
    if (rewindBuffer.empty()) {
        stopRewind();
        return;
    }

    // Load previous state
    loadRewindState();

    // Update rewind position
    currentRewindPosition = (currentRewindPosition + rewindBufferSize - 1) % rewindBufferSize;
}

void Emulator::saveRewindState() {
    if (rewindBuffer.size() >= rewindBufferSize) {
        rewindBuffer.pop();
    }

    // Save current state
    std::vector<uint8_t> state;
    // TODO: Serialize current state
    rewindBuffer.push(state);
}

void Emulator::loadRewindState() {
    if (rewindBuffer.empty()) return;

    // Load state from buffer
    std::vector<uint8_t> state = rewindBuffer.front();
    rewindBuffer.pop();
    // TODO: Deserialize state
}

void Emulator::updateAutoSave() {
    framesSinceLastSave++;
    if (framesSinceLastSave >= autoSaveInterval) {
        saveStateToFile("autosave.sav");
        framesSinceLastSave = 0;
    }
}

void Emulator::clearRewindBuffer() {
    while (!rewindBuffer.empty()) {
        rewindBuffer.pop();
    }
    currentRewindPosition = 0;
}

void Emulator::setState(EmulatorState newState) {
    std::lock_guard<std::mutex> lock(stateMutex);
    updateState(newState);
}

void Emulator::updateState(EmulatorState newState) {
    switch (newState) {
        case EmulatorState::STOPPED:
            running = false;
            paused = false;
            break;
        case EmulatorState::RUNNING:
            running = true;
            paused = false;
            break;
        case EmulatorState::PAUSED:
            running = true;
            paused = true;
            break;
        case EmulatorState::STEPPING:
            running = true;
            paused = true;
            debugStepping = true;
            break;
        case EmulatorState::DEBUGGING:
            running = true;
            paused = true;
            debugMode = true;
            break;
        case EmulatorState::REWINDING:
            running = true;
            paused = false;
            rewinding = true;
            break;
        case EmulatorState::SAVING_STATE:
            running = false;
            paused = true;
            break;
        case EmulatorState::LOADING_STATE:
            running = false;
            paused = true;
            break;
    }

    if (stateCallback) {
        stateCallback(newState);
    }
}

void Emulator::handleDebugEvent(const DebugEvent& event) {
    if (debugCallback) {
        debugCallback(event);
    }
}

void Emulator::handleError(const std::string& error) {
    if (errorCallback) {
        errorCallback(error);
    }
}

ConsoleType Emulator::detectConsoleType(const std::vector<uint8_t>& data) const {
    // TODO: Implement ROM header detection for different console types
    // This is a placeholder implementation
    if (data.size() < 4) return ConsoleType::UNKNOWN;

    // Example detection based on common ROM headers
    if (data.size() >= 0x150 && data[0x104] == 0xCE && data[0x105] == 0xED) {
        return ConsoleType::GAMEBOY;
    }
    
    if (data.size() >= 0x200 && data[0x1B] == 0x53 && data[0x1A] == 0x45) {
        return ConsoleType::GENESIS;
    }

    // Add more console detection logic here
    
    return ConsoleType::UNKNOWN;
}

std::unique_ptr<ConsoleEmulator> Emulator::createConsoleEmulator(ConsoleType type) const {
    switch (type) {
        case ConsoleType::GAMEBOY:
            return std::make_unique<GameBoyEmulator>();
        // Add more console types here as they are implemented
        default:
            std::cerr << "Unsupported console type" << std::endl;
            return nullptr;
    }
}

// Audio System Implementation
void Emulator::initializeAudio() {
    if (!audioEnabled) return;

    SDL_AudioSpec desired, obtained;
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 1024;
    desired.callback = audioCallback;
    desired.userdata = this;

    if (SDL_OpenAudio(&desired, &obtained) < 0) {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        audioEnabled = false;
        return;
    }

    SDL_PauseAudio(0);
}

void Emulator::audioCallback(void* userdata, Uint8* stream, int len) {
    Emulator* emulator = static_cast<Emulator*>(userdata);
    if (!emulator->audioEnabled) return;

    // Get audio samples from APU
    std::vector<int16_t> samples(len / 2);
    emulator->apu->getSamples(samples);

    // Mix audio channels
    for (size_t i = 0; i < samples.size(); i++) {
        samples[i] = std::clamp(samples[i], -32768, 32767);
    }

    // Copy to output buffer
    std::memcpy(stream, samples.data(), len);
}

// Video System Implementation
void Emulator::initializeVideo() {
    // Create window
    window = SDL_CreateWindow(
        "Retronexus Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160 * 4,  // Scale up GameBoy resolution
        144 * 4,
        SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0)
    );

    if (!window) {
        throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
    }

    // Create renderer
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0)
    );

    if (!renderer) {
        throw std::runtime_error("Failed to create renderer: " + std::string(SDL_GetError()));
    }

    // Create texture
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );

    if (!texture) {
        throw std::runtime_error("Failed to create texture: " + std::string(SDL_GetError()));
    }

    // Set up scaling
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, bilinearFiltering ? "1" : "0");
}

void Emulator::renderFrame() {
    if (!window || !renderer || !texture) return;

    // Get frame buffer from console
    if (console) {
        const uint8_t* frameBuffer = console->getFrameBuffer();
        if (frameBuffer) {
            // Convert frame buffer to RGBA
            std::vector<uint32_t> rgbaBuffer(160 * 144);
            for (int i = 0; i < 160 * 144; i++) {
                uint8_t color = frameBuffer[i];
                rgbaBuffer[i] = getColorFromPalette(color);
            }

            // Update texture
            SDL_UpdateTexture(texture, nullptr, rgbaBuffer.data(), 160 * sizeof(uint32_t));
        }
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render texture
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    // Render debug overlay if enabled
    if (debugMode) {
        renderDebugOverlay();
    }

    // Present frame
    SDL_RenderPresent(renderer);
}

uint32_t Emulator::getColorFromPalette(uint8_t color) const {
    // GameBoy color palette
    static const uint32_t palette[] = {
        0xFFE0F8D0,  // White
        0xFF88C070,  // Light gray
        0xFF346856,  // Dark gray
        0xFF081820   // Black
    };
    return palette[color & 0x03];
}

void Emulator::renderDebugOverlay() {
    if (!debugMode) return;

    // Create debug surface
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0,
        160 * 4,
        144 * 4,
        32,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );

    if (!surface) return;

    // Draw debug information
    SDL_Color textColor = {255, 255, 255, 255};
    TTF_Font* font = TTF_OpenFont("font.ttf", 16);
    if (font) {
        // Draw CPU registers
        std::stringstream ss;
        ss << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getPC() << "\n";
        ss << "SP: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getSP() << "\n";
        ss << "AF: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getAF() << "\n";
        ss << "BC: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getBC() << "\n";
        ss << "DE: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getDE() << "\n";
        ss << "HL: 0x" << std::hex << std::setw(4) << std::setfill('0') << console->getHL() << "\n";

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, ss.str().c_str(), textColor);
        if (textSurface) {
            SDL_Rect dest = {10, 10, textSurface->w, textSurface->h};
            SDL_BlitSurface(textSurface, nullptr, surface, &dest);
            SDL_FreeSurface(textSurface);
        }

        TTF_CloseFont(font);
    }

    // Convert surface to texture and render
    SDL_Texture* debugTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (debugTexture) {
        SDL_RenderCopy(renderer, debugTexture, nullptr, nullptr);
        SDL_DestroyTexture(debugTexture);
    }

    SDL_FreeSurface(surface);
}

// Input System Implementation
void Emulator::initializeInput() {
    // Initialize game controllers
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {
                controllers.push_back(controller);
            }
        }
    }

    // Set up input mapping
    setupDefaultInputMapping();
}

void Emulator::setupDefaultInputMapping() {
    // GameBoy button mapping
    inputMapping[SDL_SCANCODE_Z] = InputButton::A;
    inputMapping[SDL_SCANCODE_X] = InputButton::B;
    inputMapping[SDL_SCANCODE_RETURN] = InputButton::START;
    inputMapping[SDL_SCANCODE_SPACE] = InputButton::SELECT;
    inputMapping[SDL_SCANCODE_UP] = InputButton::UP;
    inputMapping[SDL_SCANCODE_DOWN] = InputButton::DOWN;
    inputMapping[SDL_SCANCODE_LEFT] = InputButton::LEFT;
    inputMapping[SDL_SCANCODE_RIGHT] = InputButton::RIGHT;

    // Controller button mapping
    controllerMapping[SDL_CONTROLLER_BUTTON_A] = InputButton::A;
    controllerMapping[SDL_CONTROLLER_BUTTON_B] = InputButton::B;
    controllerMapping[SDL_CONTROLLER_BUTTON_START] = InputButton::START;
    controllerMapping[SDL_CONTROLLER_BUTTON_BACK] = InputButton::SELECT;
    controllerMapping[SDL_CONTROLLER_BUTTON_DPAD_UP] = InputButton::UP;
    controllerMapping[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = InputButton::DOWN;
    controllerMapping[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = InputButton::LEFT;
    controllerMapping[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = InputButton::RIGHT;
}

void Emulator::setKeyState(SDL_Scancode scancode, bool pressed) {
    auto it = inputMapping.find(scancode);
    if (it != inputMapping.end()) {
        if (console) {
            console->setButtonState(it->second, pressed);
        }
    }
}

void Emulator::setControllerState(int controllerId, uint8_t button, bool pressed) {
    auto it = controllerMapping.find(button);
    if (it != controllerMapping.end()) {
        if (console) {
            console->setButtonState(it->second, pressed);
        }
    }
} 
