#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

namespace StorageOpt {

enum class USBDeviceType {
    UNKNOWN,
    USB_2_0,
    USB_3_0,
    USB_3_1,
    USB_3_2,
    USB_C
};

enum class USBOptimizationMode {
    BALANCED,       // Balance between speed and safety
    SPEED,          // Prioritize speed
    RELIABILITY,    // Prioritize data integrity
    POWER_SAVING    // Optimize for power consumption
};

struct USBDeviceInfo {
    std::string device_path;
    std::string device_name;
    std::string vendor_id;
    std::string product_id;
    USBDeviceType type;
    size_t max_transfer_size;
    size_t optimal_block_size;
    double max_throughput_mbps;
    bool supports_trim;
    bool supports_smart;
    bool is_removable;
};

struct USBTransferStats {
    size_t total_bytes_read;
    size_t total_bytes_written;
    size_t total_read_operations;
    size_t total_write_operations;
    double average_read_speed_mbps;
    double average_write_speed_mbps;
    size_t cache_hits;
    size_t cache_misses;
    std::chrono::system_clock::time_point last_access;
};

struct USBOptimizationConfig {
    size_t write_buffer_size;
    size_t read_cache_size;
    size_t optimal_transfer_size;
    bool enable_write_combining;
    bool enable_read_ahead;
    bool enable_command_queuing;
    int max_concurrent_operations;
    std::chrono::milliseconds flush_interval;
    USBOptimizationMode mode;
};

class USBDeviceDriver {
private:
    // Device management
    std::unordered_map<std::string, USBDeviceInfo> detected_devices_;
    std::unordered_map<std::string, USBTransferStats> device_stats_;
    std::mutex devices_mutex_;
    
    // Optimization buffers
    struct WriteBuffer {
        std::vector<uint8_t> data;
        size_t offset;
        std::chrono::system_clock::time_point queued_time;
        std::string device_path;
    };
    std::vector<WriteBuffer> write_buffers_;
    std::mutex write_buffer_mutex_;
    
    struct ReadCache {
        std::vector<uint8_t> data;
        size_t offset;
        std::chrono::system_clock::time_point cached_time;
        std::string device_path;
    };
    std::unordered_map<std::string, std::vector<ReadCache>> read_caches_;
    std::mutex read_cache_mutex_;
    
    // Configuration
    USBOptimizationConfig config_;
    
    // Background processing
    std::atomic<bool> running_;
    std::thread flush_thread_;
    std::thread optimization_thread_;
    std::thread monitoring_thread_;
    
    // Device-specific optimizations
    std::unordered_map<std::string, USBOptimizationConfig> device_configs_;
    
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024; // 1MB
    static constexpr size_t DEFAULT_CACHE_SIZE = 8 * 1024 * 1024; // 8MB
    static constexpr size_t USB2_OPTIMAL_SIZE = 64 * 1024; // 64KB
    static constexpr size_t USB3_OPTIMAL_SIZE = 1024 * 1024; // 1MB

public:
    USBDeviceDriver();
    ~USBDeviceDriver();
    
    // Initialization and lifecycle
    bool Initialize();
    void Start();
    void Stop();
    void Shutdown();
    
    // Device detection and management
    bool DetectUSBDevices();
    std::vector<USBDeviceInfo> GetDetectedDevices();
    USBDeviceInfo GetDeviceInfo(const std::string& device_path);
    bool IsUSBDevice(const std::string& device_path);
    USBDeviceType DetectUSBType(const std::string& device_path);
    
    // Optimized I/O operations
    bool OptimizedWrite(const std::string& device_path, size_t offset, const void* data, size_t size);
    bool OptimizedRead(const std::string& device_path, size_t offset, void* buffer, size_t size);
    bool FlushWriteBuffer(const std::string& device_path);
    bool FlushAllBuffers();
    
    // Device-specific optimizations
    bool SetOptimizationMode(const std::string& device_path, USBOptimizationMode mode);
    bool ApplyDeviceSpecificTuning(const std::string& device_path);
    bool EnableWriteCombining(const std::string& device_path, bool enable);
    bool EnableReadAhead(const std::string& device_path, bool enable);
    bool EnableCommandQueuing(const std::string& device_path, bool enable);
    
    // Performance optimization
    size_t CalculateOptimalTransferSize(const std::string& device_path);
    size_t CalculateOptimalBlockSize(const std::string& device_path);
    bool OptimizeTransferParameters(const std::string& device_path);
    double EstimateTransferTime(const std::string& device_path, size_t data_size);
    
    // Caching strategies
    bool EnableReadCache(const std::string& device_path, size_t cache_size);
    bool ClearReadCache(const std::string& device_path);
    bool PrefetchData(const std::string& device_path, size_t offset, size_t size);
    double GetCacheHitRatio(const std::string& device_path);
    
    // Statistics and monitoring
    USBTransferStats GetDeviceStats(const std::string& device_path);
    double GetAverageTransferSpeed(const std::string& device_path);
    size_t GetTotalBytesTransferred(const std::string& device_path);
    void ResetStatistics(const std::string& device_path);
    
    // Advanced features
    bool EnableUSBTurboMode(const std::string& device_path);
    bool OptimizeForSequentialAccess(const std::string& device_path);
    bool OptimizeForRandomAccess(const std::string& device_path);
    bool ReduceLatency(const std::string& device_path);
    
    // Configuration
    void SetDefaultOptimizationConfig(const USBOptimizationConfig& config);
    USBOptimizationConfig GetOptimizationConfig(const std::string& device_path);
    bool UpdateOptimizationConfig(const std::string& device_path, const USBOptimizationConfig& config);
    
    // Health and diagnostics
    bool CheckDeviceHealth(const std::string& device_path);
    std::vector<std::string> GetDeviceWarnings(const std::string& device_path);
    bool RunDiagnostics(const std::string& device_path);
    
private:
    // Background threads
    void FlushThreadLoop();
    void OptimizationThreadLoop();
    void MonitoringThreadLoop();
    
    // Device detection helpers
    bool ProbeDevice(const std::string& device_path);
    USBDeviceType IdentifyUSBVersion(const std::string& device_path);
    bool ReadDeviceDescriptor(const std::string& device_path, USBDeviceInfo& info);
    
    // Buffering and caching
    bool AddToWriteBuffer(const std::string& device_path, size_t offset, const void* data, size_t size);
    bool FlushWriteBufferInternal(const std::string& device_path);
    bool AddToReadCache(const std::string& device_path, size_t offset, const void* data, size_t size);
    bool ReadFromCache(const std::string& device_path, size_t offset, void* buffer, size_t size);
    void EvictOldCacheEntries(const std::string& device_path);
    
    // Optimization algorithms
    void AnalyzeAccessPatterns(const std::string& device_path);
    void AdjustBufferSizes(const std::string& device_path);
    void OptimizeForDeviceType(const std::string& device_path, USBDeviceType type);
    bool ShouldFlushBuffer(const WriteBuffer& buffer);
    
    // Statistics tracking
    void UpdateReadStats(const std::string& device_path, size_t bytes, double duration_ms);
    void UpdateWriteStats(const std::string& device_path, size_t bytes, double duration_ms);
    void UpdateCacheStats(const std::string& device_path, bool cache_hit);
    
    // Platform-specific implementations
#ifdef _WIN32
    bool DetectUSBDevicesWindows();
    bool ApplyWindowsOptimizations(const std::string& device_path);
#else
    bool DetectUSBDevicesLinux();
    bool ApplyLinuxOptimizations(const std::string& device_path);
#endif
    
    // Utility functions
    bool IsValidDevicePath(const std::string& device_path);
    std::string GetDeviceSerialNumber(const std::string& device_path);
    bool SupportsFeature(const std::string& device_path, const std::string& feature);
};

} // namespace StorageOpt
