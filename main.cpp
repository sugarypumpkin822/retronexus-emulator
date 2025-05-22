#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "Emulator.hpp"
#include "GameBoyEmulator.hpp"
#include "ConsoleType.hpp"
#include "PerformanceMonitor.hpp"

// Input mapping structure
struct InputMapping {
    SDL_Scancode up = SDL_SCANCODE_UP;
    SDL_Scancode down = SDL_SCANCODE_DOWN;
    SDL_Scancode left = SDL_SCANCODE_LEFT;
    SDL_Scancode right = SDL_SCANCODE_RIGHT;
    SDL_Scancode a = SDL_SCANCODE_Z;
    SDL_Scancode b = SDL_SCANCODE_X;
    SDL_Scancode start = SDL_SCANCODE_RETURN;
    SDL_Scancode select = SDL_SCANCODE_RSHIFT;
};

// Audio configuration structure
struct AudioConfig {
    int frequency = 44100;
    Uint16 format = MIX_DEFAULT_FORMAT;
    int channels = 2;
    int chunksize = 2048;
    bool enableStereo = true;
    bool enableReverb = false;
    float volume = 1.0f;
    std::string audioDevice;
};

// Global configuration settings
struct EmulatorConfig {
    bool enableAudio = true;
    bool enableSaveStates = true;
    bool enablePerformanceMonitor = true;
    bool enableCheats = true;
    bool enableDebugMode = false;
    bool enableFrameLimit = true;
    bool enableRewind = true;
    bool enableFullscreen = false;
    bool enableVSync = true;
    bool enableBilinearFiltering = true;
    int frameRate = 60;
    int windowWidth = 800;
    int windowHeight = 600;
    int rewindBufferSize = 300; // 5 seconds at 60 FPS
    std::string romPath;
    std::string saveStatePath;
    std::string configPath;
    ConsoleType consoleType;
    InputMapping inputMapping;
    AudioConfig audioConfig;
};

// Function to load configuration from file
bool loadConfig(EmulatorConfig& config, const std::string& filename) {
    // TODO: Implement configuration file loading
    return true;
}

// Function to save configuration to file
bool saveConfig(const EmulatorConfig& config, const std::string& filename) {
    // TODO: Implement configuration file saving
    return true;
}

// Function to parse command line arguments
EmulatorConfig parseArguments(int argc, char* argv[]) {
    EmulatorConfig config;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-audio") config.enableAudio = false;
        else if (arg == "--no-save-states") config.enableSaveStates = false;
        else if (arg == "--no-performance-monitor") config.enablePerformanceMonitor = false;
        else if (arg == "--no-cheats") config.enableCheats = false;
        else if (arg == "--debug") config.enableDebugMode = true;
        else if (arg == "--no-frame-limit") config.enableFrameLimit = false;
        else if (arg == "--no-rewind") config.enableRewind = false;
        else if (arg == "--fullscreen") config.enableFullscreen = true;
        else if (arg == "--no-vsync") config.enableVSync = false;
        else if (arg == "--no-filtering") config.enableBilinearFiltering = false;
        else if (arg == "--fps" && i + 1 < argc) config.frameRate = std::stoi(argv[++i]);
        else if (arg == "--width" && i + 1 < argc) config.windowWidth = std::stoi(argv[++i]);
        else if (arg == "--height" && i + 1 < argc) config.windowHeight = std::stoi(argv[++i]);
        else if (arg == "--rom" && i + 1 < argc) config.romPath = argv[++i];
        else if (arg == "--save-state" && i + 1 < argc) config.saveStatePath = argv[++i];
        else if (arg == "--config" && i + 1 < argc) config.configPath = argv[++i];
        else if (arg == "--rewind-buffer" && i + 1 < argc) config.rewindBufferSize = std::stoi(argv[++i]);
        else if (arg == "--audio-device" && i + 1 < argc) config.audioConfig.audioDevice = argv[++i];
        else if (arg == "--volume" && i + 1 < argc) config.audioConfig.volume = std::stof(argv[++i]);
        else if (arg == "--console" && i + 1 < argc) {
            std::string console = argv[++i];
            if (console == "gb") config.consoleType = ConsoleType::GAMEBOY;
            else if (console == "gbc") config.consoleType = ConsoleType::GAMEBOY_COLOR;
            else if (console == "gba") config.consoleType = ConsoleType::GAMEBOY_ADVANCE;
        }
    }
    return config;
}

// Function to initialize SDL and SDL_ttf
bool initializeSDL(const EmulatorConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    if (config.enableAudio && Mix_OpenAudio(config.audioConfig.frequency,
                                          config.audioConfig.format,
                                          config.audioConfig.channels,
                                          config.audioConfig.chunksize) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    return true;
}

// Function to create emulator instance
std::unique_ptr<Emulator> createEmulator(const EmulatorConfig& config) {
    std::unique_ptr<Emulator> emulator;
    
    switch (config.consoleType) {
        case ConsoleType::GAMEBOY:
        case ConsoleType::GAMEBOY_COLOR:
            emulator = std::make_unique<GameBoyEmulator>();
            break;
        case ConsoleType::GAMEBOY_ADVANCE:
            // TODO: Implement GBA emulator
            return nullptr;
        default:
            return nullptr;
    }
    
    if (emulator) {
        emulator->setDebugMode(config.enableDebugMode);
        emulator->setCheatsEnabled(config.enableCheats);
        emulator->setRewindEnabled(config.enableRewind);
        emulator->setRewindBufferSize(config.rewindBufferSize);
        emulator->setAudioEnabled(config.enableAudio);
        emulator->setAudioConfig(config.audioConfig);
        emulator->setInputMapping(config.inputMapping);
    }
    
    return emulator;
}

// Function to handle user input
void handleInput(Emulator& emulator, bool& running, PerformanceMonitor& monitor, const EmulatorConfig& config) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_F1:
                        emulator.saveState();
                        break;
                    case SDLK_F2:
                        emulator.loadState();
                        break;
                    case SDLK_F3:
                        emulator.togglePause();
                        break;
                    case SDLK_F4:
                        emulator.toggleFastForward();
                        break;
                    case SDLK_F5:
                        monitor.savePerformanceReport("performance_report.txt");
                        std::cout << "Performance report saved to performance_report.txt\n";
                        break;
                    case SDLK_F6:
                        std::cout << monitor.getPerformanceReport();
                        break;
                    case SDLK_F7:
                        if (config.enableDebugMode) {
                            emulator.toggleDebugger();
                        }
                        break;
                    case SDLK_F8:
                        if (config.enableRewind) {
                            emulator.startRewind();
                        }
                        break;
                    case SDLK_F9:
                        if (config.enableCheats) {
                            emulator.toggleCheatMenu();
                        }
                        break;
                    case SDLK_r:
                        if (SDL_GetModState() & KMOD_CTRL) {
                            emulator.reset();
                        }
                        break;
                    case SDLK_f:
                        if (SDL_GetModState() & KMOD_CTRL) {
                            emulator.toggleFullscreen();
                        }
                        break;
                    case SDLK_s:
                        if (SDL_GetModState() & KMOD_CTRL) {
                            saveConfig(config, config.configPath);
                        }
                        break;
                }
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_F8 && config.enableRewind) {
                    emulator.stopRewind();
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                // TODO: Implement game controller support
                break;
            case SDL_CONTROLLERBUTTONUP:
                // TODO: Implement game controller support
                break;
        }
    }
}

// Function to render text using SDL_ttf
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, 
                int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dest = {x, y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dest);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

// Function to run the emulator
void runEmulator(std::unique_ptr<Emulator>& emulator, const EmulatorConfig& config) {
    bool running = true;
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    const auto frameDuration = std::chrono::microseconds(1000000 / config.frameRate);
    
    // Initialize performance monitor
    PerformanceMonitor monitor;
    
    // Create performance display window
    SDL_Window* perfWindow = nullptr;
    SDL_Renderer* perfRenderer = nullptr;
    TTF_Font* perfFont = nullptr;
    
    if (config.enablePerformanceMonitor) {
        perfWindow = SDL_CreateWindow("Performance Monitor",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    300, 200, SDL_WINDOW_SHOWN);
        if (perfWindow) {
            perfRenderer = SDL_CreateRenderer(perfWindow, -1, SDL_RENDERER_ACCELERATED);
            if (perfRenderer) {
                perfFont = TTF_OpenFont("arial.ttf", 16);
                if (!perfFont) {
                    std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
                }
            }
        }
    }

    while (running) {
        monitor.startFrame();
        monitor.startCPUMeasurement();
        
        handleInput(*emulator, running, monitor, config);
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);
        
        if (!config.enableFrameLimit || elapsed >= frameDuration) {
            monitor.startGPUMeasurement();
            emulator->runFrame();
            monitor.endGPUMeasurement();
            lastFrameTime = currentTime;
        } else if (config.enableFrameLimit) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
        
        monitor.endCPUMeasurement();
        monitor.endFrame();
        
        // Update performance display
        if (config.enablePerformanceMonitor && perfWindow && perfRenderer && perfFont) {
            SDL_SetRenderDrawColor(perfRenderer, 0, 0, 0, 255);
            SDL_RenderClear(perfRenderer);
            
            SDL_Color textColor = {255, 255, 255, 255};
            
            // Draw FPS
            std::string fpsText = "FPS: " + std::to_string(static_cast<int>(monitor.getCurrentFPS()));
            renderText(perfRenderer, perfFont, fpsText, 10, 10, textColor);
            
            // Draw CPU usage
            std::string cpuText = "CPU: " + std::to_string(static_cast<int>(monitor.getCPUUsage())) + "%";
            renderText(perfRenderer, perfFont, cpuText, 10, 40, textColor);
            
            // Draw GPU usage
            std::string gpuText = "GPU: " + std::to_string(static_cast<int>(monitor.getGPUUsage())) + "%";
            renderText(perfRenderer, perfFont, gpuText, 10, 70, textColor);
            
            // Draw memory usage
            std::string memText = "Memory: " + std::to_string(static_cast<int>(monitor.getMemoryUsagePercentage())) + "%";
            renderText(perfRenderer, perfFont, memText, 10, 100, textColor);
            
            // Draw alerts if any
            if (monitor.hasAlerts()) {
                auto alerts = monitor.getAlerts();
                for (size_t i = 0; i < alerts.size(); ++i) {
                    renderText(perfRenderer, perfFont, alerts[i], 10, 130 + i * 20, {255, 0, 0, 255});
                }
            }
            
            SDL_RenderPresent(perfRenderer);
        }
    }
    
    // Cleanup performance monitor window
    if (perfFont) TTF_CloseFont(perfFont);
    if (perfRenderer) SDL_DestroyRenderer(perfRenderer);
    if (perfWindow) SDL_DestroyWindow(perfWindow);
}

// Function to display advanced configuration UI
void showConfigUI(EmulatorConfig& config) {
    std::cout << "--- Emulator Configuration ---\n";
    std::cout << "1. Audio: " << (config.enableAudio ? "Enabled" : "Disabled") << "\n";
    std::cout << "2. Save States: " << (config.enableSaveStates ? "Enabled" : "Disabled") << "\n";
    std::cout << "3. Performance Monitor: " << (config.enablePerformanceMonitor ? "Enabled" : "Disabled") << "\n";
    std::cout << "4. Cheats: " << (config.enableCheats ? "Enabled" : "Disabled") << "\n";
    std::cout << "5. Debug Mode: " << (config.enableDebugMode ? "Enabled" : "Disabled") << "\n";
    std::cout << "6. Frame Limit: " << (config.enableFrameLimit ? "Enabled" : "Disabled") << "\n";
    std::cout << "7. Rewind: " << (config.enableRewind ? "Enabled" : "Disabled") << "\n";
    std::cout << "8. Fullscreen: " << (config.enableFullscreen ? "Enabled" : "Disabled") << "\n";
    std::cout << "9. VSync: " << (config.enableVSync ? "Enabled" : "Disabled") << "\n";
    std::cout << "10. Bilinear Filtering: " << (config.enableBilinearFiltering ? "Enabled" : "Disabled") << "\n";
    std::cout << "11. Frame Rate: " << config.frameRate << "\n";
    std::cout << "12. Window Size: " << config.windowWidth << "x" << config.windowHeight << "\n";
    std::cout << "13. ROM Path: " << config.romPath << "\n";
    std::cout << "14. Save State Path: " << config.saveStatePath << "\n";
    std::cout << "15. Config Path: " << config.configPath << "\n";
    std::cout << "16. Console Type: " << static_cast<int>(config.consoleType) << "\n";
    std::cout << "17. Input Mapping: (not shown)\n";
    std::cout << "18. Audio Config: (not shown)\n";
    std::cout << "Enter number to toggle/modify, or 0 to exit: ";
    int choice;
    std::cin >> choice;
    switch (choice) {
        case 1: config.enableAudio = !config.enableAudio; break;
        case 2: config.enableSaveStates = !config.enableSaveStates; break;
        case 3: config.enablePerformanceMonitor = !config.enablePerformanceMonitor; break;
        case 4: config.enableCheats = !config.enableCheats; break;
        case 5: config.enableDebugMode = !config.enableDebugMode; break;
        case 6: config.enableFrameLimit = !config.enableFrameLimit; break;
        case 7: config.enableRewind = !config.enableRewind; break;
        case 8: config.enableFullscreen = !config.enableFullscreen; break;
        case 9: config.enableVSync = !config.enableVSync; break;
        case 10: config.enableBilinearFiltering = !config.enableBilinearFiltering; break;
        case 11: std::cout << "Enter new frame rate: "; std::cin >> config.frameRate; break;
        case 12: std::cout << "Enter width: "; std::cin >> config.windowWidth; std::cout << "Enter height: "; std::cin >> config.windowHeight; break;
        case 13: std::cout << "Enter ROM path: "; std::cin >> config.romPath; break;
        case 14: std::cout << "Enter save state path: "; std::cin >> config.saveStatePath; break;
        case 15: std::cout << "Enter config path: "; std::cin >> config.configPath; break;
        case 16: std::cout << "Enter console type (0=GB,1=GBC,2=GBA): "; int t; std::cin >> t; config.consoleType = static_cast<ConsoleType>(t); break;
        default: break;
    }
}

// Function to initialize game controller support
bool initializeControllers() {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL GameController init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Controller connected: " << SDL_GameControllerName(controller) << "\n";
            } else {
                std::cerr << "Could not open controller: " << SDL_GetError() << "\n";
            }
        }
    }
    return true;
}

// Enhanced error handling for ROM loading
bool tryLoadROM(Emulator& emulator, const std::string& path) {
    if (!emulator.loadROM(path)) {
        std::cerr << "Error: Failed to load ROM: " << path << "\n";
        std::cerr << "Please check the file path and try again.\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    EmulatorConfig config = parseArguments(argc, argv);
    if (!config.configPath.empty()) {
        loadConfig(config, config.configPath);
    }
    if (!initializeSDL(config)) {
        return 1;
    }
    initializeControllers();
    auto emulator = createEmulator(config);
    if (!emulator) {
        std::cerr << "Failed to create emulator instance" << std::endl;
        SDL_Quit();
        return 1;
    }
    if (!tryLoadROM(*emulator, config.romPath)) {
        SDL_Quit();
        return 1;
    }
    if (!config.saveStatePath.empty()) {
        emulator->loadState(config.saveStatePath);
    }
    // Show config UI before running
    showConfigUI(config);
    runEmulator(emulator, config);
    if (config.enableAudio) {
        Mix_CloseAudio();
    }
    TTF_Quit();
    SDL_Quit();
    return 0;
} 
