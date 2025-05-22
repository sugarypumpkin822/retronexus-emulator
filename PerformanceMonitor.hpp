#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();

    // FPS monitoring
    void startFrame();
    void endFrame();
    double getCurrentFPS() const;
    double getAverageFPS() const;
    double getMinFPS() const;
    double getMaxFPS() const;
    double getFrameTime() const;
    double getFrameTimeVariance() const;
    double getFrameTimePercentile(double percentile) const;
    double getFrameTimeJitter() const;

    // CPU monitoring
    void startCPUMeasurement();
    void endCPUMeasurement();
    double getCPUUsage() const;
    double getAverageCPUUsage() const;
    double getMinCPUUsage() const;
    double getMaxCPUUsage() const;
    double getCPUTemperature() const;
    int getCPUThreadCount() const;
    double getCPUClockSpeed() const;
    double getCPUPowerUsage() const;
    std::vector<double> getCPUUsagePerCore() const;

    // GPU monitoring
    void startGPUMeasurement();
    void endGPUMeasurement();
    double getGPUUsage() const;
    double getAverageGPUUsage() const;
    double getMinGPUUsage() const;
    double getMaxGPUUsage() const;
    double getGPUTemperature() const;
    int getGPUMemoryUsage() const;
    double getGPUClockSpeed() const;
    double getGPUPowerUsage() const;
    int getGPUFanSpeed() const;

    // Memory monitoring
    size_t getTotalMemoryUsage() const;
    size_t getPeakMemoryUsage() const;
    double getMemoryUsagePercentage() const;
    size_t getAvailableMemory() const;
    size_t getTotalSystemMemory() const;
    double getMemoryFragmentation() const;
    size_t getSwapUsage() const;
    size_t getCacheUsage() const;
    double getMemoryBandwidth() const;

    // Network monitoring
    void startNetworkMonitoring();
    void stopNetworkMonitoring();
    double getNetworkBandwidth() const;
    double getNetworkLatency() const;
    size_t getNetworkPacketsSent() const;
    size_t getNetworkPacketsReceived() const;
    size_t getNetworkBytesSent() const;
    size_t getNetworkBytesReceived() const;
    double getNetworkErrorRate() const;

    // Performance history
    void setHistorySize(size_t size);
    const std::deque<double>& getFPSHistory() const;
    const std::deque<double>& getCPUHistory() const;
    const std::deque<double>& getGPUHistory() const;
    const std::deque<double>& getMemoryHistory() const;
    const std::deque<double>& getNetworkHistory() const;
    void clearHistory();
    void exportHistory(const std::string& filename) const;
    void importHistory(const std::string& filename);

    // Performance reporting
    std::string getPerformanceReport() const;
    void savePerformanceReport(const std::string& filename) const;
    void exportPerformanceData(const std::string& filename) const;
    void setReportFormat(const std::string& format);
    void setReportInterval(int frames);
    void enableAutoReporting(bool enable);
    void setAutoReportPath(const std::string& path);

    // Performance alerts
    void setFPSAlert(double threshold);
    void setCPUAlert(double threshold);
    void setGPUAlert(double threshold);
    void setMemoryAlert(double threshold);
    void setNetworkAlert(double threshold);
    bool hasAlerts() const;
    std::vector<std::string> getAlerts() const;
    void clearAlerts();
    void setAlertCallback(std::function<void(const std::string&)> callback);

    // Performance graphs
    void enableGraphs(bool enable);
    void setGraphUpdateInterval(int frames);
    void setGraphStyle(const std::string& style);
    void renderGraphs(SDL_Renderer* renderer, int x, int y, int width, int height);
    void setGraphColors(const std::vector<SDL_Color>& colors);
    void setGraphBackground(const SDL_Color& color);
    void setGraphGrid(bool enable);
    void setGraphLegend(bool enable);

    // Advanced statistics
    void calculateStatistics();
    double getStandardDeviation(const std::deque<double>& data) const;
    double getMedian(const std::deque<double>& data) const;
    double getMode(const std::deque<double>& data) const;
    double getSkewness(const std::deque<double>& data) const;
    double getKurtosis(const std::deque<double>& data) const;
    std::vector<double> getOutliers(const std::deque<double>& data) const;
    void detectAnomalies();
    std::vector<std::string> getAnomalies() const;

    // Logging and session statistics
    void enableLogging(bool enable);
    void setLogFile(const std::string& filename);
    void logEvent(const std::string& event);
    void startSession();
    void endSession();
    double getSessionUptime() const;
    size_t getSessionFrameCount() const;
    double getSessionAverageFPS() const;
    std::string getSessionSummary() const;

    // Plugin hooks for custom metrics
    void registerCustomMetric(const std::string& name, std::function<double()> getter);
    double getCustomMetric(const std::string& name) const;
    std::vector<std::string> getCustomMetricNames() const;

    // More graphing options
    void setGraphType(const std::string& type); // e.g., "line", "bar", "scatter"
    void setGraphSmoothing(bool enable);
    void setGraphYAxisRange(double min, double max);
    void setGraphTitle(const std::string& title);
    void setGraphFont(TTF_Font* font);
    void setGraphLabelColor(const SDL_Color& color);
    void setGraphDataColor(const SDL_Color& color);
    void setGraphShowDataPoints(bool enable);
    void setGraphShowAverageLine(bool enable);
    void setGraphShowMinMax(bool enable);

    // Logging and plugin state
    bool isLoggingEnabled() const;
    std::string getLogFile() const;
    std::vector<std::string> getLogHistory() const;
    bool isSessionActive() const;
    std::map<std::string, double> getAllCustomMetrics() const;

private:
    // FPS tracking
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    std::deque<double> frameTimes;
    std::deque<double> fpsHistory;
    double currentFPS;
    double averageFPS;
    double minFPS;
    double maxFPS;
    double frameTimeVariance;
    double frameTimeJitter;

    // CPU tracking
    std::chrono::high_resolution_clock::time_point lastCPUTime;
    std::deque<double> cpuTimes;
    std::deque<double> cpuHistory;
    double currentCPUUsage;
    double averageCPUUsage;
    double minCPUUsage;
    double maxCPUUsage;
    double cpuTemperature;
    int cpuThreadCount;
    double cpuClockSpeed;
    double cpuPowerUsage;
    std::vector<double> cpuUsagePerCore;

    // GPU tracking
    std::chrono::high_resolution_clock::time_point lastGPUTime;
    std::deque<double> gpuTimes;
    std::deque<double> gpuHistory;
    double currentGPUUsage;
    double averageGPUUsage;
    double minGPUUsage;
    double maxGPUUsage;
    double gpuTemperature;
    int gpuMemoryUsage;
    double gpuClockSpeed;
    double gpuPowerUsage;
    int gpuFanSpeed;

    // Memory tracking
    size_t totalMemoryUsage;
    size_t peakMemoryUsage;
    double memoryUsagePercentage;
    size_t availableMemory;
    size_t totalSystemMemory;
    double memoryFragmentation;
    size_t swapUsage;
    size_t cacheUsage;
    double memoryBandwidth;
    std::deque<double> memoryHistory;

    // Network tracking
    bool networkMonitoring;
    double networkBandwidth;
    double networkLatency;
    size_t networkPacketsSent;
    size_t networkPacketsReceived;
    size_t networkBytesSent;
    size_t networkBytesReceived;
    double networkErrorRate;
    std::deque<double> networkHistory;
    IPaddress serverAddress;
    TCPsocket serverSocket;

    // Configuration
    size_t historySize;
    std::mutex monitorMutex;
    std::atomic<bool> isMonitoring;
    std::string reportFormat;
    bool graphsEnabled;
    int graphUpdateInterval;
    std::string graphStyle;
    std::vector<SDL_Color> graphColors;
    SDL_Color graphBackground;
    bool graphGrid;
    bool graphLegend;
    int reportInterval;
    bool autoReporting;
    std::string autoReportPath;

    // Alert thresholds
    double fpsAlertThreshold;
    double cpuAlertThreshold;
    double gpuAlertThreshold;
    double memoryAlertThreshold;
    double networkAlertThreshold;
    std::vector<std::string> activeAlerts;
    std::function<void(const std::string&)> alertCallback;

    // Statistics
    std::vector<std::string> anomalies;
    std::vector<double> outliers;

    // Helper functions
    void updateStatistics();
    void calculateAverages();
    void updateMinMax();
    void trimHistory();
    void initializeMonitoring();
    void cleanupMonitoring();
    void resetStatistics();
    void updateMemoryUsage();
    void updateCPUUsage();
    void updateGPUUsage();
    void updateFPS();
    void updateHistory();
    void updateReport();
    void saveReport(const std::string& filename) const;
    void checkAlerts();
    void updateGraphs();
    void renderGraph(SDL_Renderer* renderer, const std::deque<double>& data,
                    int x, int y, int width, int height, const std::string& label);
    std::string formatPerformanceData() const;
    std::string formatMemoryData() const;
    std::string formatTimingData() const;
    std::string formatUsageData() const;
    std::string formatHistoryData() const;
    std::string formatReport() const;
    std::string formatAlert(const std::string& type, double value, double threshold) const;
    void initializeNetworkMonitoring();
    void cleanupNetworkMonitoring();
    void updateNetworkStats();
    void detectNetworkIssues();
    void handleNetworkError();
    void exportNetworkData(const std::string& filename) const;
    void importNetworkData(const std::string& filename);
    void calculateAdvancedStatistics();
    void detectPerformanceAnomalies();
    void handleAnomaly(const std::string& type, double value, double threshold);
    void updateGraphColors();
    void renderGraphLegend(SDL_Renderer* renderer, int x, int y);
    void renderGraphGrid(SDL_Renderer* renderer, int x, int y, int width, int height);

    // Logging
    bool loggingEnabled = false;
    std::string logFile;
    std::vector<std::string> logHistory;
    // Session statistics
    std::chrono::high_resolution_clock::time_point sessionStartTime;
    std::chrono::high_resolution_clock::time_point sessionEndTime;
    size_t sessionFrameCount = 0;
    double sessionAverageFPS = 0.0;
    bool sessionActive = false;
    // Plugin hooks
    std::map<std::string, std::function<double()>> customMetrics;
    std::map<std::string, double> customMetricCache;
    // More graphing options
    std::string graphType = "line";
    bool graphSmoothing = false;
    double graphYAxisMin = 0.0;
    double graphYAxisMax = 100.0;
    std::string graphTitle;
    TTF_Font* graphFont = nullptr;
    SDL_Color graphLabelColor = {255,255,255,255};
    SDL_Color graphDataColor = {0,255,0,255};
    bool graphShowDataPoints = false;
    bool graphShowAverageLine = false;
    bool graphShowMinMax = false;
}; 
