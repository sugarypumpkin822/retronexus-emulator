#include "PerformanceMonitor.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include <cmath>

PerformanceMonitor::PerformanceMonitor()
    : historySize(100)
    , currentFPS(0.0)
    , averageFPS(0.0)
    , minFPS(std::numeric_limits<double>::max())
    , maxFPS(0.0)
    , frameTimeVariance(0.0)
    , frameTimeJitter(0.0)
    , currentCPUUsage(0.0)
    , averageCPUUsage(0.0)
    , minCPUUsage(std::numeric_limits<double>::max())
    , maxCPUUsage(0.0)
    , cpuTemperature(0.0)
    , cpuThreadCount(std::thread::hardware_concurrency())
    , cpuClockSpeed(0.0)
    , cpuPowerUsage(0.0)
    , currentGPUUsage(0.0)
    , averageGPUUsage(0.0)
    , minGPUUsage(std::numeric_limits<double>::max())
    , maxGPUUsage(0.0)
    , gpuTemperature(0.0)
    , gpuMemoryUsage(0)
    , gpuClockSpeed(0.0)
    , gpuPowerUsage(0.0)
    , gpuFanSpeed(0)
    , totalMemoryUsage(0)
    , peakMemoryUsage(0)
    , memoryUsagePercentage(0.0)
    , availableMemory(0)
    , totalSystemMemory(0)
    , memoryFragmentation(0.0)
    , swapUsage(0)
    , cacheUsage(0)
    , memoryBandwidth(0.0)
    , networkMonitoring(false)
    , networkBandwidth(0.0)
    , networkLatency(0.0)
    , networkPacketsSent(0)
    , networkPacketsReceived(0)
    , networkBytesSent(0)
    , networkBytesReceived(0)
    , networkErrorRate(0.0)
    , isMonitoring(false)
    , reportFormat("text")
    , graphsEnabled(false)
    , graphUpdateInterval(1)
    , graphStyle("line")
    , graphBackground({0, 0, 0, 255})
    , graphGrid(false)
    , graphLegend(false)
    , reportInterval(60)
    , autoReporting(false)
    , fpsAlertThreshold(30.0)
    , cpuAlertThreshold(90.0)
    , gpuAlertThreshold(90.0)
    , memoryAlertThreshold(90.0)
    , networkAlertThreshold(1000.0) // 1 second latency threshold
    , loggingEnabled(false)
    , logFile("")
    , sessionActive(false)
{
    initializeMonitoring();
    initializeNetworkMonitoring();
    updateGraphColors();
}

PerformanceMonitor::~PerformanceMonitor() {
    cleanupNetworkMonitoring();
    cleanupMonitoring();
}

void PerformanceMonitor::startFrame() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    lastFrameTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endFrame() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime).count() / 1000000.0;
    
    frameTimes.push_back(frameTime);
    if (frameTimes.size() > historySize) {
        frameTimes.pop_front();
    }
    
    updateFPS();
    updateStatistics();
    checkAlerts();
    detectAnomalies();
    
    if (graphsEnabled && frameTimes.size() % graphUpdateInterval == 0) {
        updateGraphs();
    }
    
    if (autoReporting && frameTimes.size() % reportInterval == 0) {
        savePerformanceReport(autoReportPath);
    }
}

double PerformanceMonitor::getFrameTime() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return frameTimes.empty() ? 0.0 : frameTimes.back();
}

double PerformanceMonitor::getFrameTimeVariance() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return frameTimeVariance;
}

double PerformanceMonitor::getFrameTimePercentile(double percentile) const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    if (frameTimes.empty()) return 0.0;
    
    std::vector<double> sortedTimes(frameTimes.begin(), frameTimes.end());
    std::sort(sortedTimes.begin(), sortedTimes.end());
    
    size_t index = static_cast<size_t>(percentile * sortedTimes.size() / 100.0);
    return sortedTimes[index];
}

double PerformanceMonitor::getFrameTimeJitter() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return frameTimeJitter;
}

void PerformanceMonitor::startCPUMeasurement() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    lastCPUTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endCPUMeasurement() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto cpuTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastCPUTime).count() / 1000000.0;
    
    cpuTimes.push_back(cpuTime);
    if (cpuTimes.size() > historySize) {
        cpuTimes.pop_front();
    }
    
    updateCPUUsage();
}

double PerformanceMonitor::getCPUTemperature() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return cpuTemperature;
}

int PerformanceMonitor::getCPUThreadCount() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return cpuThreadCount;
}

void PerformanceMonitor::startGPUMeasurement() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    lastGPUTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endGPUMeasurement() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto gpuTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastGPUTime).count() / 1000000.0;
    
    gpuTimes.push_back(gpuTime);
    if (gpuTimes.size() > historySize) {
        gpuTimes.pop_front();
    }
    
    updateGPUUsage();
}

double PerformanceMonitor::getGPUTemperature() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return gpuTemperature;
}

int PerformanceMonitor::getGPUMemoryUsage() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return gpuMemoryUsage;
}

void PerformanceMonitor::updateFPS() {
    if (frameTimes.empty()) return;
    
    currentFPS = 1.0 / frameTimes.back();
    fpsHistory.push_back(currentFPS);
    if (fpsHistory.size() > historySize) {
        fpsHistory.pop_front();
    }
    
    minFPS = std::min(minFPS, currentFPS);
    maxFPS = std::max(maxFPS, currentFPS);
    averageFPS = std::accumulate(fpsHistory.begin(), fpsHistory.end(), 0.0) / fpsHistory.size();
    
    // Calculate frame time variance
    double mean = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
    double sumSquaredDiff = 0.0;
    for (double time : frameTimes) {
        sumSquaredDiff += (time - mean) * (time - mean);
    }
    frameTimeVariance = sumSquaredDiff / frameTimes.size();
}

void PerformanceMonitor::updateCPUUsage() {
    if (cpuTimes.empty()) return;
    
    currentCPUUsage = cpuTimes.back() * 100.0;
    cpuHistory.push_back(currentCPUUsage);
    if (cpuHistory.size() > historySize) {
        cpuHistory.pop_front();
    }
    
    minCPUUsage = std::min(minCPUUsage, currentCPUUsage);
    maxCPUUsage = std::max(maxCPUUsage, currentCPUUsage);
    averageCPUUsage = std::accumulate(cpuHistory.begin(), cpuHistory.end(), 0.0) / cpuHistory.size();
    
    // Update CPU temperature (platform specific)
    // TODO: Implement platform-specific CPU temperature monitoring
}

void PerformanceMonitor::updateGPUUsage() {
    if (gpuTimes.empty()) return;
    
    currentGPUUsage = gpuTimes.back() * 100.0;
    gpuHistory.push_back(currentGPUUsage);
    if (gpuHistory.size() > historySize) {
        gpuHistory.pop_front();
    }
    
    minGPUUsage = std::min(minGPUUsage, currentGPUUsage);
    maxGPUUsage = std::max(maxGPUUsage, currentGPUUsage);
    averageGPUUsage = std::accumulate(gpuHistory.begin(), gpuHistory.end(), 0.0) / gpuHistory.size();
    
    // Update GPU temperature and memory usage (platform specific)
    // TODO: Implement platform-specific GPU monitoring
}

void PerformanceMonitor::updateMemoryUsage() {
    // Get process memory usage
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        totalMemoryUsage = pmc.WorkingSetSize;
        peakMemoryUsage = std::max(peakMemoryUsage, totalMemoryUsage);
        
        // Calculate memory usage percentage
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        memoryUsagePercentage = (double)totalMemoryUsage / memInfo.ullTotalPhys * 100.0;
        availableMemory = memInfo.ullAvailPhys;
        totalSystemMemory = memInfo.ullTotalPhys;
        
        // Calculate memory fragmentation
        // TODO: Implement memory fragmentation calculation
        
        memoryHistory.push_back(memoryUsagePercentage);
        if (memoryHistory.size() > historySize) {
            memoryHistory.pop_front();
        }
    }
}

void PerformanceMonitor::checkAlerts() {
    activeAlerts.clear();
    
    if (currentFPS < fpsAlertThreshold) {
        activeAlerts.push_back(formatAlert("FPS", currentFPS, fpsAlertThreshold));
    }
    if (currentCPUUsage > cpuAlertThreshold) {
        activeAlerts.push_back(formatAlert("CPU", currentCPUUsage, cpuAlertThreshold));
    }
    if (currentGPUUsage > gpuAlertThreshold) {
        activeAlerts.push_back(formatAlert("GPU", currentGPUUsage, gpuAlertThreshold));
    }
    if (memoryUsagePercentage > memoryAlertThreshold) {
        activeAlerts.push_back(formatAlert("Memory", memoryUsagePercentage, memoryAlertThreshold));
    }
}

void PerformanceMonitor::renderGraphs(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!graphsEnabled || !renderer) return;
    
    int graphHeight = height / 4;
    int spacing = 10;
    
    // Render FPS graph
    renderGraph(renderer, fpsHistory, x, y, width, graphHeight, "FPS");
    
    // Render CPU usage graph
    renderGraph(renderer, cpuHistory, x, y + graphHeight + spacing, width, graphHeight, "CPU");
    
    // Render GPU usage graph
    renderGraph(renderer, gpuHistory, x, y + (graphHeight + spacing) * 2, width, graphHeight, "GPU");
    
    // Render Memory usage graph
    renderGraph(renderer, memoryHistory, x, y + (graphHeight + spacing) * 3, width, graphHeight, "Memory");
}

void PerformanceMonitor::renderGraph(SDL_Renderer* renderer, const std::deque<double>& data,
                                   int x, int y, int width, int height, const std::string& label) {
    if (data.empty()) return;
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect bgRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Draw label
    // TODO: Implement text rendering for label
    
    // Draw graph
    if (graphStyle == "line") {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int prevX = x;
        int prevY = y + height;
        
        for (size_t i = 0; i < data.size(); ++i) {
            int currentX = x + (i * width) / (data.size() - 1);
            int currentY = y + height - (data[i] * height) / 100.0;
            
            SDL_RenderDrawLine(renderer, prevX, prevY, currentX, currentY);
            
            prevX = currentX;
            prevY = currentY;
        }
    } else if (graphStyle == "bar") {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int barWidth = width / data.size();
        
        for (size_t i = 0; i < data.size(); ++i) {
            int barHeight = (data[i] * height) / 100.0;
            SDL_Rect barRect = {x + i * barWidth, y + height - barHeight, barWidth, barHeight};
            SDL_RenderFillRect(renderer, &barRect);
        }
    }
}

std::string PerformanceMonitor::formatAlert(const std::string& type, double value, double threshold) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << type << " alert: " << value;
    if (type == "FPS") {
        ss << " FPS (below threshold of " << threshold << " FPS)";
    } else {
        ss << "% (above threshold of " << threshold << "%)";
    }
    return ss.str();
}

void PerformanceMonitor::exportPerformanceData(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Time,FPS,CPU,GPU,Memory\n";
        
        for (size_t i = 0; i < fpsHistory.size(); ++i) {
            file << i << ","
                 << fpsHistory[i] << ","
                 << cpuHistory[i] << ","
                 << gpuHistory[i] << ","
                 << memoryHistory[i] << "\n";
        }
        
        file.close();
    }
}

void PerformanceMonitor::setReportFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    reportFormat = format;
}

void PerformanceMonitor::enableGraphs(bool enable) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    graphsEnabled = enable;
}

void PerformanceMonitor::setGraphUpdateInterval(int frames) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    graphUpdateInterval = frames;
}

void PerformanceMonitor::setGraphStyle(const std::string& style) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    graphStyle = style;
}

void PerformanceMonitor::setFPSAlert(double threshold) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    fpsAlertThreshold = threshold;
}

void PerformanceMonitor::setCPUAlert(double threshold) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    cpuAlertThreshold = threshold;
}

void PerformanceMonitor::setGPUAlert(double threshold) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    gpuAlertThreshold = threshold;
}

void PerformanceMonitor::setMemoryAlert(double threshold) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    memoryAlertThreshold = threshold;
}

bool PerformanceMonitor::hasAlerts() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return !activeAlerts.empty();
}

std::vector<std::string> PerformanceMonitor::getAlerts() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return activeAlerts;
}

void PerformanceMonitor::clearAlerts() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    activeAlerts.clear();
}

void PerformanceMonitor::clearHistory() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    frameTimes.clear();
    fpsHistory.clear();
    cpuTimes.clear();
    cpuHistory.clear();
    gpuTimes.clear();
    gpuHistory.clear();
    memoryHistory.clear();
    resetStatistics();
}

void PerformanceMonitor::initializeMonitoring() {
    isMonitoring = true;
    resetStatistics();
}

void PerformanceMonitor::cleanupMonitoring() {
    isMonitoring = false;
}

void PerformanceMonitor::resetStatistics() {
    frameTimes.clear();
    fpsHistory.clear();
    cpuTimes.clear();
    cpuHistory.clear();
    gpuTimes.clear();
    gpuHistory.clear();
    
    currentFPS = 0.0;
    averageFPS = 0.0;
    minFPS = std::numeric_limits<double>::max();
    maxFPS = 0.0;
    
    currentCPUUsage = 0.0;
    averageCPUUsage = 0.0;
    minCPUUsage = std::numeric_limits<double>::max();
    maxCPUUsage = 0.0;
    
    currentGPUUsage = 0.0;
    averageGPUUsage = 0.0;
    minGPUUsage = std::numeric_limits<double>::max();
    maxGPUUsage = 0.0;
    
    totalMemoryUsage = 0;
    peakMemoryUsage = 0;
    memoryUsagePercentage = 0.0;
}

void PerformanceMonitor::startNetworkMonitoring() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    if (!networkMonitoring) {
        networkMonitoring = true;
        // TODO: Implement network monitoring initialization
    }
}

void PerformanceMonitor::stopNetworkMonitoring() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    if (networkMonitoring) {
        networkMonitoring = false;
        // TODO: Implement network monitoring cleanup
    }
}

double PerformanceMonitor::getNetworkBandwidth() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return networkBandwidth;
}

double PerformanceMonitor::getNetworkLatency() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    return networkLatency;
}

void PerformanceMonitor::updateNetworkStats() {
    if (!networkMonitoring) return;
    
    // TODO: Implement network statistics update
    // This would involve measuring network latency, bandwidth, and packet statistics
}

void PerformanceMonitor::detectNetworkIssues() {
    if (!networkMonitoring) return;
    
    if (networkLatency > networkAlertThreshold) {
        handleAnomaly("Network", networkLatency, networkAlertThreshold);
    }
    
    if (networkErrorRate > 0.01) { // 1% error rate threshold
        handleAnomaly("Network", networkErrorRate, 0.01);
    }
}

void PerformanceMonitor::calculateAdvancedStatistics() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    
    // Calculate frame time statistics
    frameTimeJitter = getStandardDeviation(frameTimes);
    
    // Calculate CPU statistics
    cpuClockSpeed = 0.0; // TODO: Implement CPU clock speed detection
    cpuPowerUsage = 0.0; // TODO: Implement CPU power usage detection
    
    // Calculate GPU statistics
    gpuClockSpeed = 0.0; // TODO: Implement GPU clock speed detection
    gpuPowerUsage = 0.0; // TODO: Implement GPU power usage detection
    gpuFanSpeed = 0; // TODO: Implement GPU fan speed detection
    
    // Calculate memory statistics
    memoryBandwidth = 0.0; // TODO: Implement memory bandwidth detection
    swapUsage = 0; // TODO: Implement swap usage detection
    cacheUsage = 0; // TODO: Implement cache usage detection
}

double PerformanceMonitor::getStandardDeviation(const std::deque<double>& data) const {
    if (data.empty()) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double sumSquaredDiff = 0.0;
    
    for (double value : data) {
        sumSquaredDiff += (value - mean) * (value - mean);
    }
    
    return std::sqrt(sumSquaredDiff / data.size());
}

double PerformanceMonitor::getMedian(const std::deque<double>& data) const {
    if (data.empty()) return 0.0;
    
    std::vector<double> sortedData(data.begin(), data.end());
    std::sort(sortedData.begin(), sortedData.end());
    
    size_t mid = sortedData.size() / 2;
    if (sortedData.size() % 2 == 0) {
        return (sortedData[mid - 1] + sortedData[mid]) / 2.0;
    } else {
        return sortedData[mid];
    }
}

double PerformanceMonitor::getMode(const std::deque<double>& data) const {
    if (data.empty()) return 0.0;
    
    std::map<double, int> frequency;
    for (double value : data) {
        frequency[value]++;
    }
    
    double mode = data[0];
    int maxFrequency = 0;
    
    for (const auto& pair : frequency) {
        if (pair.second > maxFrequency) {
            maxFrequency = pair.second;
            mode = pair.first;
        }
    }
    
    return mode;
}

double PerformanceMonitor::getSkewness(const std::deque<double>& data) const {
    if (data.size() < 3) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double stdDev = getStandardDeviation(data);
    
    if (stdDev == 0.0) return 0.0;
    
    double sumCubedDiff = 0.0;
    for (double value : data) {
        double diff = (value - mean) / stdDev;
        sumCubedDiff += diff * diff * diff;
    }
    
    return (sumCubedDiff / data.size()) * std::sqrt(data.size() * (data.size() - 1)) / (data.size() - 2);
}

double PerformanceMonitor::getKurtosis(const std::deque<double>& data) const {
    if (data.size() < 4) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double stdDev = getStandardDeviation(data);
    
    if (stdDev == 0.0) return 0.0;
    
    double sumQuarticDiff = 0.0;
    for (double value : data) {
        double diff = (value - mean) / stdDev;
        sumQuarticDiff += diff * diff * diff * diff;
    }
    
    return (sumQuarticDiff / data.size()) - 3.0;
}

std::vector<double> PerformanceMonitor::getOutliers(const std::deque<double>& data) const {
    if (data.size() < 4) return {};
    
    double q1 = getFrameTimePercentile(25);
    double q3 = getFrameTimePercentile(75);
    double iqr = q3 - q1;
    double lowerBound = q1 - 1.5 * iqr;
    double upperBound = q3 + 1.5 * iqr;
    
    std::vector<double> outliers;
    for (double value : data) {
        if (value < lowerBound || value > upperBound) {
            outliers.push_back(value);
        }
    }
    
    return outliers;
}

void PerformanceMonitor::detectAnomalies() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    anomalies.clear();
    
    // Detect FPS anomalies
    if (currentFPS < fpsAlertThreshold) {
        handleAnomaly("FPS", currentFPS, fpsAlertThreshold);
    }
    
    // Detect CPU anomalies
    if (currentCPUUsage > cpuAlertThreshold) {
        handleAnomaly("CPU", currentCPUUsage, cpuAlertThreshold);
    }
    
    // Detect GPU anomalies
    if (currentGPUUsage > gpuAlertThreshold) {
        handleAnomaly("GPU", currentGPUUsage, gpuAlertThreshold);
    }
    
    // Detect memory anomalies
    if (memoryUsagePercentage > memoryAlertThreshold) {
        handleAnomaly("Memory", memoryUsagePercentage, memoryAlertThreshold);
    }
    
    // Detect network anomalies
    if (networkMonitoring) {
        detectNetworkIssues();
    }
}

void PerformanceMonitor::handleAnomaly(const std::string& type, double value, double threshold) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << type << " anomaly detected: " << value;
    if (type == "FPS") {
        ss << " FPS (below threshold of " << threshold << " FPS)";
    } else {
        ss << "% (above threshold of " << threshold << "%)";
    }
    anomalies.push_back(ss.str());
    
    if (alertCallback) {
        alertCallback(ss.str());
    }
}

void PerformanceMonitor::updateGraphColors() {
    graphColors = {
        {0, 255, 0, 255},   // Green for FPS
        {255, 0, 0, 255},   // Red for CPU
        {0, 0, 255, 255},   // Blue for GPU
        {255, 255, 0, 255}, // Yellow for Memory
        {255, 0, 255, 255}  // Magenta for Network
    };
}

void PerformanceMonitor::renderGraphLegend(SDL_Renderer* renderer, int x, int y) {
    if (!graphLegend || !renderer) return;
    
    const std::vector<std::string> labels = {"FPS", "CPU", "GPU", "Memory", "Network"};
    int spacing = 20;
    
    for (size_t i = 0; i < labels.size(); ++i) {
        SDL_SetRenderDrawColor(renderer, graphColors[i].r, graphColors[i].g, graphColors[i].b, graphColors[i].a);
        SDL_Rect colorRect = {x, y + i * spacing, 10, 10};
        SDL_RenderFillRect(renderer, &colorRect);
        
        // TODO: Render label text
    }
}

void PerformanceMonitor::renderGraphGrid(SDL_Renderer* renderer, int x, int y, int width, int height) {
    if (!graphGrid || !renderer) return;
    
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    
    // Draw vertical grid lines
    for (int i = 0; i <= 10; ++i) {
        int xPos = x + (width * i) / 10;
        SDL_RenderDrawLine(renderer, xPos, y, xPos, y + height);
    }
    
    // Draw horizontal grid lines
    for (int i = 0; i <= 5; ++i) {
        int yPos = y + (height * i) / 5;
        SDL_RenderDrawLine(renderer, x, yPos, x + width, yPos);
    }
}

// Logging and session statistics
void PerformanceMonitor::enableLogging(bool enable) {
    loggingEnabled = enable;
}
void PerformanceMonitor::setLogFile(const std::string& filename) {
    logFile = filename;
}
void PerformanceMonitor::logEvent(const std::string& event) {
    if (!loggingEnabled) return;
    logHistory.push_back(event);
    if (!logFile.empty()) {
        std::ofstream file(logFile, std::ios::app);
        if (file.is_open()) {
            file << event << std::endl;
        }
    }
}
void PerformanceMonitor::startSession() {
    sessionStartTime = std::chrono::high_resolution_clock::now();
    sessionFrameCount = 0;
    sessionAverageFPS = 0.0;
    sessionActive = true;
    logEvent("Session started");
}
void PerformanceMonitor::endSession() {
    sessionEndTime = std::chrono::high_resolution_clock::now();
    sessionActive = false;
    logEvent("Session ended");
}
double PerformanceMonitor::getSessionUptime() const {
    if (!sessionActive) return std::chrono::duration<double>(sessionEndTime - sessionStartTime).count();
    return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - sessionStartTime).count();
}
size_t PerformanceMonitor::getSessionFrameCount() const {
    return sessionFrameCount;
}
double PerformanceMonitor::getSessionAverageFPS() const {
    return sessionFrameCount > 0 ? sessionAverageFPS : 0.0;
}
std::string PerformanceMonitor::getSessionSummary() const {
    std::stringstream ss;
    ss << "Session Uptime: " << getSessionUptime() << "s\n";
    ss << "Frames: " << getSessionFrameCount() << "\n";
    ss << "Average FPS: " << getSessionAverageFPS() << "\n";
    return ss.str();
}

// Plugin hooks for custom metrics
void PerformanceMonitor::registerCustomMetric(const std::string& name, std::function<double()> getter) {
    customMetrics[name] = getter;
}
double PerformanceMonitor::getCustomMetric(const std::string& name) const {
    auto it = customMetrics.find(name);
    if (it != customMetrics.end()) {
        return it->second();
    }
    auto cacheIt = customMetricCache.find(name);
    if (cacheIt != customMetricCache.end()) {
        return cacheIt->second;
    }
    return 0.0;
}
std::vector<std::string> PerformanceMonitor::getCustomMetricNames() const {
    std::vector<std::string> names;
    for (const auto& pair : customMetrics) {
        names.push_back(pair.first);
    }
    return names;
}

// More graphing options
void PerformanceMonitor::setGraphType(const std::string& type) {
    graphType = type;
}
void PerformanceMonitor::setGraphSmoothing(bool enable) {
    graphSmoothing = enable;
}
void PerformanceMonitor::setGraphYAxisRange(double min, double max) {
    graphYAxisMin = min;
    graphYAxisMax = max;
}
void PerformanceMonitor::setGraphTitle(const std::string& title) {
    graphTitle = title;
}
void PerformanceMonitor::setGraphFont(TTF_Font* font) {
    graphFont = font;
}
void PerformanceMonitor::setGraphLabelColor(const SDL_Color& color) {
    graphLabelColor = color;
}
void PerformanceMonitor::setGraphDataColor(const SDL_Color& color) {
    graphDataColor = color;
}
void PerformanceMonitor::setGraphShowDataPoints(bool enable) {
    graphShowDataPoints = enable;
}
void PerformanceMonitor::setGraphShowAverageLine(bool enable) {
    graphShowAverageLine = enable;
}
void PerformanceMonitor::setGraphShowMinMax(bool enable) {
    graphShowMinMax = enable;
}
bool PerformanceMonitor::isLoggingEnabled() const {
    return loggingEnabled;
}
std::string PerformanceMonitor::getLogFile() const {
    return logFile;
}
std::vector<std::string> PerformanceMonitor::getLogHistory() const {
    return logHistory;
}
bool PerformanceMonitor::isSessionActive() const {
    return sessionActive;
}
std::map<std::string, double> PerformanceMonitor::getAllCustomMetrics() const {
    std::map<std::string, double> result;
    for (const auto& pair : customMetrics) {
        result[pair.first] = pair.second();
    }
    return result;
} 
