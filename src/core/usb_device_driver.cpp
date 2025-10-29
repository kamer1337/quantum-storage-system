#include "usb_device_driver.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

namespace StorageOpt {

USBDeviceDriver::USBDeviceDriver() 
    : running_(false) {
    // Initialize default configuration
    config_.write_buffer_size = DEFAULT_BUFFER_SIZE;
    config_.read_cache_size = DEFAULT_CACHE_SIZE;
    config_.optimal_transfer_size = USB3_OPTIMAL_SIZE;
    config_.enable_write_combining = true;
    config_.enable_read_ahead = true;
    config_.enable_command_queuing = true;
    config_.max_concurrent_operations = 4;
    config_.flush_interval = std::chrono::milliseconds(1000);
    config_.mode = USBOptimizationMode::BALANCED;
}

USBDeviceDriver::~USBDeviceDriver() {
    Shutdown();
}

bool USBDeviceDriver::Initialize() {
    std::cout << "[USB Driver] Initializing USB Device Driver..." << std::endl;
    
    // Detect USB devices
    if (!DetectUSBDevices()) {
        std::cerr << "[USB Driver] Warning: Failed to detect USB devices" << std::endl;
    }
    
    std::cout << "[USB Driver] Detected " << detected_devices_.size() << " USB device(s)" << std::endl;
    
    // Initialize device-specific configurations
    for (const auto& [path, info] : detected_devices_) {
        USBOptimizationConfig device_config = config_;
        OptimizeForDeviceType(path, info.type);
        device_configs_[path] = device_config;
        
        // Initialize statistics
        device_stats_[path] = USBTransferStats{};
        device_stats_[path].last_access = std::chrono::system_clock::now();
    }
    
    return true;
}

void USBDeviceDriver::Start() {
    std::cout << "[USB Driver] Starting USB optimization services..." << std::endl;
    running_ = true;
    
    // Start background threads
    flush_thread_ = std::thread(&USBDeviceDriver::FlushThreadLoop, this);
    optimization_thread_ = std::thread(&USBDeviceDriver::OptimizationThreadLoop, this);
    monitoring_thread_ = std::thread(&USBDeviceDriver::MonitoringThreadLoop, this);
    
    std::cout << "[USB Driver] USB optimization services started" << std::endl;
}

void USBDeviceDriver::Stop() {
    std::cout << "[USB Driver] Stopping USB optimization services..." << std::endl;
    running_ = false;
    
    // Flush all pending writes
    FlushAllBuffers();
    
    // Wait for threads to finish
    if (flush_thread_.joinable()) flush_thread_.join();
    if (optimization_thread_.joinable()) optimization_thread_.join();
    if (monitoring_thread_.joinable()) monitoring_thread_.join();
    
    std::cout << "[USB Driver] USB optimization services stopped" << std::endl;
}

void USBDeviceDriver::Shutdown() {
    Stop();
    
    // Clear all caches and buffers
    {
        std::lock_guard<std::mutex> lock(write_buffer_mutex_);
        write_buffers_.clear();
    }
    
    {
        std::lock_guard<std::mutex> lock(read_cache_mutex_);
        read_caches_.clear();
    }
    
    // Clear device information
    {
        std::lock_guard<std::mutex> lock(devices_mutex_);
        detected_devices_.clear();
        device_stats_.clear();
        device_configs_.clear();
    }
}

bool USBDeviceDriver::DetectUSBDevices() {
#ifdef _WIN32
    return DetectUSBDevicesWindows();
#else
    return DetectUSBDevicesLinux();
#endif
}

#ifdef _WIN32
bool USBDeviceDriver::DetectUSBDevicesWindows() {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    // Simulate USB device detection on Windows
    // In a real implementation, this would use Windows API to detect USB devices
    std::cout << "[USB Driver] Scanning for USB devices on Windows..." << std::endl;
    
    // Example: Add a simulated USB device
    USBDeviceInfo device;
    device.device_path = "\\\\.\\PHYSICALDRIVE1";
    device.device_name = "USB Storage Device";
    device.vendor_id = "0x1234";
    device.product_id = "0x5678";
    device.type = USBDeviceType::USB_3_0;
    device.max_transfer_size = 1024 * 1024; // 1MB
    device.optimal_block_size = 64 * 1024; // 64KB
    device.max_throughput_mbps = 500.0;
    device.supports_trim = true;
    device.supports_smart = false;
    device.is_removable = true;
    
    detected_devices_[device.device_path] = device;
    
    return true;
}

bool USBDeviceDriver::ApplyWindowsOptimizations(const std::string& device_path) {
    // Windows-specific optimizations
    std::cout << "[USB Driver] Applying Windows optimizations for " << device_path << std::endl;
    
    // Would use Windows API to:
    // - Disable write caching if needed
    // - Set optimal buffer sizes
    // - Configure power management
    
    return true;
}
#else
bool USBDeviceDriver::DetectUSBDevicesLinux() {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    std::cout << "[USB Driver] Scanning for USB devices on Linux..." << std::endl;
    
    // Scan /sys/bus/usb/devices for USB storage devices
    DIR* dir = opendir("/sys/bus/usb/devices");
    if (!dir) {
        // If USB directory doesn't exist, create simulated device for testing
        USBDeviceInfo device;
        device.device_path = "/dev/sdb";
        device.device_name = "USB Storage Device";
        device.vendor_id = "0x1234";
        device.product_id = "0x5678";
        device.type = USBDeviceType::USB_3_0;
        device.max_transfer_size = 1024 * 1024;
        device.optimal_block_size = 64 * 1024;
        device.max_throughput_mbps = 500.0;
        device.supports_trim = true;
        device.supports_smart = false;
        device.is_removable = true;
        
        detected_devices_[device.device_path] = device;
        return true;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        
        std::string device_path = std::string("/sys/bus/usb/devices/") + entry->d_name;
        
        // Check if this is a storage device
        std::string bDeviceClass_path = device_path + "/bDeviceClass";
        std::ifstream bDeviceClass_file(bDeviceClass_path);
        if (!bDeviceClass_file.is_open()) continue;
        
        std::string device_class;
        std::getline(bDeviceClass_file, device_class);
        
        // USB Mass Storage class is 08
        if (device_class == "08" || device_class == "08h") {
            USBDeviceInfo device;
            device.device_path = device_path;
            device.device_name = std::string("USB Device ") + entry->d_name;
            device.type = DetectUSBType(device_path);
            device.max_transfer_size = 1024 * 1024;
            device.optimal_block_size = device.type == USBDeviceType::USB_2_0 ? USB2_OPTIMAL_SIZE : USB3_OPTIMAL_SIZE;
            device.max_throughput_mbps = device.type == USBDeviceType::USB_2_0 ? 60.0 : 500.0;
            device.supports_trim = true;
            device.supports_smart = false;
            device.is_removable = true;
            
            detected_devices_[device.device_path] = device;
        }
    }
    
    closedir(dir);
    return true;
}

bool USBDeviceDriver::ApplyLinuxOptimizations(const std::string& device_path) {
    std::cout << "[USB Driver] Applying Linux optimizations for " << device_path << std::endl;
    
    // Linux-specific optimizations:
    // - Adjust /sys/block/*/queue/read_ahead_kb
    // - Configure scheduler
    // - Set optimal I/O parameters
    
    return true;
}
#endif

std::vector<USBDeviceInfo> USBDeviceDriver::GetDetectedDevices() {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    std::vector<USBDeviceInfo> devices;
    for (const auto& [path, info] : detected_devices_) {
        devices.push_back(info);
    }
    return devices;
}

USBDeviceInfo USBDeviceDriver::GetDeviceInfo(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    auto it = detected_devices_.find(device_path);
    if (it != detected_devices_.end()) {
        return it->second;
    }
    return USBDeviceInfo{};
}

bool USBDeviceDriver::IsUSBDevice(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    return detected_devices_.find(device_path) != detected_devices_.end();
}

USBDeviceType USBDeviceDriver::DetectUSBType(const std::string& device_path) {
    // Read USB version from device descriptor
    // This is a simplified implementation
    
#ifndef _WIN32
    std::string version_path = device_path + "/version";
    std::ifstream version_file(version_path);
    if (version_file.is_open()) {
        std::string version;
        std::getline(version_file, version);
        
        // Parse version string (e.g., " 3.00", " 2.00")
        double ver = std::stod(version);
        if (ver >= 3.2) return USBDeviceType::USB_3_2;
        if (ver >= 3.1) return USBDeviceType::USB_3_1;
        if (ver >= 3.0) return USBDeviceType::USB_3_0;
        if (ver >= 2.0) return USBDeviceType::USB_2_0;
    }
#endif
    
    // Default to USB 3.0 for this demo
    return USBDeviceType::USB_3_0;
}

bool USBDeviceDriver::OptimizedWrite(const std::string& device_path, size_t offset, const void* data, size_t size) {
    if (!IsUSBDevice(device_path)) {
        std::cerr << "[USB Driver] Device not found: " << device_path << std::endl;
        return false;
    }
    
    auto config = GetOptimizationConfig(device_path);
    
    if (config.enable_write_combining) {
        // Add to write buffer for combining
        return AddToWriteBuffer(device_path, offset, data, size);
    } else {
        // Direct write
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Simulate write operation
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long>(size / 100.0)));
        
        auto end_time = std::chrono::high_resolution_clock::now();
        double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        UpdateWriteStats(device_path, size, duration_ms);
        return true;
    }
}

bool USBDeviceDriver::OptimizedRead(const std::string& device_path, size_t offset, void* buffer, size_t size) {
    if (!IsUSBDevice(device_path)) {
        std::cerr << "[USB Driver] Device not found: " << device_path << std::endl;
        return false;
    }
    
    auto config = GetOptimizationConfig(device_path);
    
    // Try to read from cache first
    if (config.enable_read_ahead && ReadFromCache(device_path, offset, buffer, size)) {
        UpdateCacheStats(device_path, true);
        return true;
    }
    
    UpdateCacheStats(device_path, false);
    
    // Perform actual read
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Simulate read operation
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long>(size / 150.0)));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    UpdateReadStats(device_path, size, duration_ms);
    
    // Add to cache if read-ahead is enabled
    if (config.enable_read_ahead) {
        AddToReadCache(device_path, offset, buffer, size);
    }
    
    return true;
}

bool USBDeviceDriver::FlushWriteBuffer(const std::string& device_path) {
    return FlushWriteBufferInternal(device_path);
}

bool USBDeviceDriver::FlushAllBuffers() {
    std::lock_guard<std::mutex> lock(write_buffer_mutex_);
    
    std::cout << "[USB Driver] Flushing all write buffers..." << std::endl;
    
    // Group buffers by device
    std::unordered_map<std::string, std::vector<WriteBuffer>> device_buffers;
    for (const auto& buffer : write_buffers_) {
        device_buffers[buffer.device_path].push_back(buffer);
    }
    
    // Flush each device's buffers
    for (const auto& [device_path, buffers] : device_buffers) {
        for (const auto& buffer : buffers) {
            // Simulate actual write
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    
    write_buffers_.clear();
    return true;
}

bool USBDeviceDriver::SetOptimizationMode(const std::string& device_path, USBOptimizationMode mode) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    auto& config = device_configs_[device_path];
    config.mode = mode;
    
    // Adjust parameters based on mode
    switch (mode) {
        case USBOptimizationMode::SPEED:
            config.write_buffer_size = DEFAULT_BUFFER_SIZE * 2;
            config.read_cache_size = DEFAULT_CACHE_SIZE * 2;
            config.enable_write_combining = true;
            config.enable_read_ahead = true;
            config.max_concurrent_operations = 8;
            break;
            
        case USBOptimizationMode::RELIABILITY:
            config.write_buffer_size = DEFAULT_BUFFER_SIZE / 2;
            config.flush_interval = std::chrono::milliseconds(500);
            config.enable_write_combining = false;
            config.max_concurrent_operations = 2;
            break;
            
        case USBOptimizationMode::POWER_SAVING:
            config.write_buffer_size = DEFAULT_BUFFER_SIZE * 2;
            config.flush_interval = std::chrono::milliseconds(2000);
            config.enable_command_queuing = false;
            config.max_concurrent_operations = 2;
            break;
            
        case USBOptimizationMode::BALANCED:
        default:
            config.write_buffer_size = DEFAULT_BUFFER_SIZE;
            config.read_cache_size = DEFAULT_CACHE_SIZE;
            config.enable_write_combining = true;
            config.enable_read_ahead = true;
            config.max_concurrent_operations = 4;
            break;
    }
    
    std::cout << "[USB Driver] Set optimization mode for " << device_path << " to " 
              << static_cast<int>(mode) << std::endl;
    
    return true;
}

bool USBDeviceDriver::ApplyDeviceSpecificTuning(const std::string& device_path) {
    auto info = GetDeviceInfo(device_path);
    if (info.device_path.empty()) {
        return false;
    }
    
    OptimizeForDeviceType(device_path, info.type);
    
#ifdef _WIN32
    ApplyWindowsOptimizations(device_path);
#else
    ApplyLinuxOptimizations(device_path);
#endif
    
    return true;
}

size_t USBDeviceDriver::CalculateOptimalTransferSize(const std::string& device_path) {
    auto info = GetDeviceInfo(device_path);
    if (info.device_path.empty()) {
        return USB3_OPTIMAL_SIZE;
    }
    
    // Calculate based on device type
    switch (info.type) {
        case USBDeviceType::USB_2_0:
            return USB2_OPTIMAL_SIZE;
        case USBDeviceType::USB_3_0:
        case USBDeviceType::USB_3_1:
        case USBDeviceType::USB_3_2:
        case USBDeviceType::USB_C:
            return USB3_OPTIMAL_SIZE;
        default:
            return USB3_OPTIMAL_SIZE;
    }
}

size_t USBDeviceDriver::CalculateOptimalBlockSize(const std::string& device_path) {
    auto info = GetDeviceInfo(device_path);
    if (info.device_path.empty()) {
        return 64 * 1024; // Default 64KB
    }
    
    return info.optimal_block_size;
}

bool USBDeviceDriver::EnableReadCache(const std::string& device_path, size_t cache_size) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    device_configs_[device_path].read_cache_size = cache_size;
    device_configs_[device_path].enable_read_ahead = true;
    
    std::cout << "[USB Driver] Enabled read cache for " << device_path 
              << " with size " << cache_size / 1024 / 1024 << " MB" << std::endl;
    
    return true;
}

bool USBDeviceDriver::ClearReadCache(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(read_cache_mutex_);
    
    auto it = read_caches_.find(device_path);
    if (it != read_caches_.end()) {
        it->second.clear();
        return true;
    }
    
    return false;
}

USBTransferStats USBDeviceDriver::GetDeviceStats(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    auto it = device_stats_.find(device_path);
    if (it != device_stats_.end()) {
        return it->second;
    }
    
    return USBTransferStats{};
}

double USBDeviceDriver::GetCacheHitRatio(const std::string& device_path) {
    auto stats = GetDeviceStats(device_path);
    size_t total_accesses = stats.cache_hits + stats.cache_misses;
    
    if (total_accesses == 0) {
        return 0.0;
    }
    
    return static_cast<double>(stats.cache_hits) / total_accesses;
}

bool USBDeviceDriver::EnableUSBTurboMode(const std::string& device_path) {
    std::cout << "[USB Driver] Enabling USB Turbo Mode for " << device_path << std::endl;
    
    // Set to maximum performance
    SetOptimizationMode(device_path, USBOptimizationMode::SPEED);
    
    auto& config = device_configs_[device_path];
    config.write_buffer_size = DEFAULT_BUFFER_SIZE * 4;
    config.read_cache_size = DEFAULT_CACHE_SIZE * 4;
    config.max_concurrent_operations = 16;
    
    return true;
}

bool USBDeviceDriver::OptimizeForSequentialAccess(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    auto& config = device_configs_[device_path];
    config.enable_read_ahead = true;
    config.enable_write_combining = true;
    config.optimal_transfer_size = USB3_OPTIMAL_SIZE * 2; // Larger transfers for sequential
    
    std::cout << "[USB Driver] Optimized " << device_path << " for sequential access" << std::endl;
    return true;
}

bool USBDeviceDriver::OptimizeForRandomAccess(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    auto& config = device_configs_[device_path];
    config.enable_read_ahead = false; // Less useful for random access
    config.enable_command_queuing = true; // More important for random
    config.optimal_transfer_size = USB2_OPTIMAL_SIZE; // Smaller transfers
    
    std::cout << "[USB Driver] Optimized " << device_path << " for random access" << std::endl;
    return true;
}

void USBDeviceDriver::SetDefaultOptimizationConfig(const USBOptimizationConfig& config) {
    config_ = config;
}

USBOptimizationConfig USBDeviceDriver::GetOptimizationConfig(const std::string& device_path) {
    auto it = device_configs_.find(device_path);
    if (it != device_configs_.end()) {
        return it->second;
    }
    return config_;
}

bool USBDeviceDriver::UpdateOptimizationConfig(const std::string& device_path, const USBOptimizationConfig& config) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    device_configs_[device_path] = config;
    return true;
}

// Background thread implementations
void USBDeviceDriver::FlushThreadLoop() {
    while (running_) {
        // Sleep for flush interval
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Check and flush buffers that need flushing
        std::lock_guard<std::mutex> lock(write_buffer_mutex_);
        
        auto now = std::chrono::system_clock::now();
        for (auto it = write_buffers_.begin(); it != write_buffers_.end();) {
            if (ShouldFlushBuffer(*it)) {
                // Flush this buffer
                it = write_buffers_.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void USBDeviceDriver::OptimizationThreadLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Analyze access patterns and optimize
        std::lock_guard<std::mutex> lock(devices_mutex_);
        for (const auto& [device_path, info] : detected_devices_) {
            AnalyzeAccessPatterns(device_path);
            AdjustBufferSizes(device_path);
        }
    }
}

void USBDeviceDriver::MonitoringThreadLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // Monitor device health and performance
        std::lock_guard<std::mutex> lock(devices_mutex_);
        for (const auto& [device_path, info] : detected_devices_) {
            // Could perform health checks here
        }
    }
}

// Private helper implementations
bool USBDeviceDriver::AddToWriteBuffer(const std::string& device_path, size_t offset, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(write_buffer_mutex_);
    
    WriteBuffer buffer;
    buffer.device_path = device_path;
    buffer.offset = offset;
    buffer.data.resize(size);
    std::memcpy(buffer.data.data(), data, size);
    buffer.queued_time = std::chrono::system_clock::now();
    
    write_buffers_.push_back(std::move(buffer));
    
    // Check if we should flush immediately
    auto config = GetOptimizationConfig(device_path);
    if (write_buffers_.size() * size >= config.write_buffer_size) {
        return FlushWriteBufferInternal(device_path);
    }
    
    return true;
}

bool USBDeviceDriver::FlushWriteBufferInternal(const std::string& device_path) {
    // Simulate flushing writes
    std::cout << "[USB Driver] Flushing write buffer for " << device_path << std::endl;
    return true;
}

bool USBDeviceDriver::AddToReadCache(const std::string& device_path, size_t offset, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(read_cache_mutex_);
    
    ReadCache cache;
    cache.device_path = device_path;
    cache.offset = offset;
    cache.data.resize(size);
    std::memcpy(cache.data.data(), data, size);
    cache.cached_time = std::chrono::system_clock::now();
    
    read_caches_[device_path].push_back(std::move(cache));
    
    // Evict old entries if cache is too large
    EvictOldCacheEntries(device_path);
    
    return true;
}

bool USBDeviceDriver::ReadFromCache(const std::string& device_path, size_t offset, void* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(read_cache_mutex_);
    
    auto it = read_caches_.find(device_path);
    if (it == read_caches_.end()) {
        return false;
    }
    
    // Look for matching cache entry
    for (const auto& cache : it->second) {
        if (cache.offset == offset && cache.data.size() >= size) {
            std::memcpy(buffer, cache.data.data(), size);
            return true;
        }
    }
    
    return false;
}

void USBDeviceDriver::EvictOldCacheEntries(const std::string& device_path) {
    auto& caches = read_caches_[device_path];
    auto config = GetOptimizationConfig(device_path);
    
    // Calculate total cache size
    size_t total_size = 0;
    for (const auto& cache : caches) {
        total_size += cache.data.size();
    }
    
    // Evict oldest entries if over limit
    while (total_size > config.read_cache_size && !caches.empty()) {
        total_size -= caches.front().data.size();
        caches.erase(caches.begin());
    }
}

void USBDeviceDriver::AnalyzeAccessPatterns(const std::string& device_path) {
    // Analyze read/write patterns to optimize configuration
    auto stats = GetDeviceStats(device_path);
    
    // Could adjust optimization based on patterns
    // For now, this is a placeholder
}

void USBDeviceDriver::AdjustBufferSizes(const std::string& device_path) {
    // Dynamically adjust buffer sizes based on performance
    auto stats = GetDeviceStats(device_path);
    
    // Could adjust based on observed performance
    // For now, this is a placeholder
}

void USBDeviceDriver::OptimizeForDeviceType(const std::string& device_path, USBDeviceType type) {
    auto& config = device_configs_[device_path];
    
    switch (type) {
        case USBDeviceType::USB_2_0:
            config.optimal_transfer_size = USB2_OPTIMAL_SIZE;
            config.write_buffer_size = DEFAULT_BUFFER_SIZE / 2;
            config.max_concurrent_operations = 2;
            break;
            
        case USBDeviceType::USB_3_0:
        case USBDeviceType::USB_3_1:
        case USBDeviceType::USB_3_2:
        case USBDeviceType::USB_C:
            config.optimal_transfer_size = USB3_OPTIMAL_SIZE;
            config.write_buffer_size = DEFAULT_BUFFER_SIZE;
            config.max_concurrent_operations = 8;
            break;
            
        default:
            break;
    }
}

bool USBDeviceDriver::ShouldFlushBuffer(const WriteBuffer& buffer) {
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - buffer.queued_time);
    
    auto config = GetOptimizationConfig(buffer.device_path);
    return age >= config.flush_interval;
}

void USBDeviceDriver::UpdateReadStats(const std::string& device_path, size_t bytes, double duration_ms) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    auto& stats = device_stats_[device_path];
    stats.total_bytes_read += bytes;
    stats.total_read_operations++;
    stats.last_access = std::chrono::system_clock::now();
    
    // Update average speed (moving average)
    double speed_mbps = (bytes / 1024.0 / 1024.0) / (duration_ms / 1000.0);
    if (stats.average_read_speed_mbps == 0.0) {
        stats.average_read_speed_mbps = speed_mbps;
    } else {
        stats.average_read_speed_mbps = 0.9 * stats.average_read_speed_mbps + 0.1 * speed_mbps;
    }
}

void USBDeviceDriver::UpdateWriteStats(const std::string& device_path, size_t bytes, double duration_ms) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    auto& stats = device_stats_[device_path];
    stats.total_bytes_written += bytes;
    stats.total_write_operations++;
    stats.last_access = std::chrono::system_clock::now();
    
    // Update average speed (moving average)
    double speed_mbps = (bytes / 1024.0 / 1024.0) / (duration_ms / 1000.0);
    if (stats.average_write_speed_mbps == 0.0) {
        stats.average_write_speed_mbps = speed_mbps;
    } else {
        stats.average_write_speed_mbps = 0.9 * stats.average_write_speed_mbps + 0.1 * speed_mbps;
    }
}

void USBDeviceDriver::UpdateCacheStats(const std::string& device_path, bool cache_hit) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    
    auto& stats = device_stats_[device_path];
    if (cache_hit) {
        stats.cache_hits++;
    } else {
        stats.cache_misses++;
    }
}

bool USBDeviceDriver::EnableWriteCombining(const std::string& device_path, bool enable) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    device_configs_[device_path].enable_write_combining = enable;
    return true;
}

bool USBDeviceDriver::EnableReadAhead(const std::string& device_path, bool enable) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    device_configs_[device_path].enable_read_ahead = enable;
    return true;
}

bool USBDeviceDriver::EnableCommandQueuing(const std::string& device_path, bool enable) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    device_configs_[device_path].enable_command_queuing = enable;
    return true;
}

bool USBDeviceDriver::CheckDeviceHealth(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    // Simulated health check
    std::cout << "[USB Driver] Checking health of " << device_path << "... OK" << std::endl;
    return true;
}

std::vector<std::string> USBDeviceDriver::GetDeviceWarnings(const std::string& device_path) {
    std::vector<std::string> warnings;
    
    auto stats = GetDeviceStats(device_path);
    
    // Check for slow performance
    if (stats.average_write_speed_mbps < 10.0 && stats.total_write_operations > 10) {
        warnings.push_back("Write speed is slower than expected");
    }
    
    if (stats.average_read_speed_mbps < 10.0 && stats.total_read_operations > 10) {
        warnings.push_back("Read speed is slower than expected");
    }
    
    // Check cache effectiveness
    double hit_ratio = GetCacheHitRatio(device_path);
    if (hit_ratio < 0.3 && (stats.cache_hits + stats.cache_misses) > 100) {
        warnings.push_back("Low cache hit ratio - consider adjusting cache size");
    }
    
    return warnings;
}

bool USBDeviceDriver::PrefetchData(const std::string& device_path, size_t offset, size_t size) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    // Simulate prefetch
    std::cout << "[USB Driver] Prefetching " << size << " bytes at offset " << offset 
              << " for " << device_path << std::endl;
    
    return true;
}

double USBDeviceDriver::GetAverageTransferSpeed(const std::string& device_path) {
    auto stats = GetDeviceStats(device_path);
    return (stats.average_read_speed_mbps + stats.average_write_speed_mbps) / 2.0;
}

size_t USBDeviceDriver::GetTotalBytesTransferred(const std::string& device_path) {
    auto stats = GetDeviceStats(device_path);
    return stats.total_bytes_read + stats.total_bytes_written;
}

void USBDeviceDriver::ResetStatistics(const std::string& device_path) {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    device_stats_[device_path] = USBTransferStats{};
}

bool USBDeviceDriver::ReduceLatency(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    auto& config = device_configs_[device_path];
    config.flush_interval = std::chrono::milliseconds(100); // Faster flush
    config.enable_command_queuing = true;
    config.max_concurrent_operations = 16;
    
    std::cout << "[USB Driver] Reduced latency settings applied to " << device_path << std::endl;
    return true;
}

bool USBDeviceDriver::OptimizeTransferParameters(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        return false;
    }
    
    auto info = GetDeviceInfo(device_path);
    auto& config = device_configs_[device_path];
    
    config.optimal_transfer_size = CalculateOptimalTransferSize(device_path);
    OptimizeForDeviceType(device_path, info.type);
    
    std::cout << "[USB Driver] Optimized transfer parameters for " << device_path << std::endl;
    return true;
}

double USBDeviceDriver::EstimateTransferTime(const std::string& device_path, size_t data_size) {
    auto info = GetDeviceInfo(device_path);
    if (info.device_path.empty()) {
        return 0.0;
    }
    
    // Estimate based on max throughput
    double size_mb = data_size / 1024.0 / 1024.0;
    double time_seconds = size_mb / info.max_throughput_mbps;
    
    return time_seconds;
}

bool USBDeviceDriver::RunDiagnostics(const std::string& device_path) {
    if (!IsUSBDevice(device_path)) {
        std::cerr << "[USB Driver] Device not found: " << device_path << std::endl;
        return false;
    }
    
    std::cout << "\n=== USB Device Diagnostics ===" << std::endl;
    std::cout << "Device: " << device_path << std::endl;
    
    auto info = GetDeviceInfo(device_path);
    std::cout << "Name: " << info.device_name << std::endl;
    std::cout << "Type: " << static_cast<int>(info.type) << std::endl;
    std::cout << "Max throughput: " << info.max_throughput_mbps << " MB/s" << std::endl;
    
    auto stats = GetDeviceStats(device_path);
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "Total bytes read: " << stats.total_bytes_read / 1024 / 1024 << " MB" << std::endl;
    std::cout << "Total bytes written: " << stats.total_bytes_written / 1024 / 1024 << " MB" << std::endl;
    std::cout << "Average read speed: " << stats.average_read_speed_mbps << " MB/s" << std::endl;
    std::cout << "Average write speed: " << stats.average_write_speed_mbps << " MB/s" << std::endl;
    std::cout << "Cache hit ratio: " << GetCacheHitRatio(device_path) * 100 << "%" << std::endl;
    
    auto warnings = GetDeviceWarnings(device_path);
    if (!warnings.empty()) {
        std::cout << "\nWarnings:" << std::endl;
        for (const auto& warning : warnings) {
            std::cout << "- " << warning << std::endl;
        }
    } else {
        std::cout << "\nNo warnings - device operating normally" << std::endl;
    }
    
    return true;
}

bool USBDeviceDriver::IsValidDevicePath(const std::string& device_path) {
    return !device_path.empty();
}

std::string USBDeviceDriver::GetDeviceSerialNumber(const std::string& device_path) {
    // Would read actual serial number from device
    return "SN123456789";
}

bool USBDeviceDriver::SupportsFeature(const std::string& device_path, const std::string& feature) {
    auto info = GetDeviceInfo(device_path);
    if (info.device_path.empty()) {
        return false;
    }
    
    if (feature == "TRIM") return info.supports_trim;
    if (feature == "SMART") return info.supports_smart;
    
    return false;
}

} // namespace StorageOpt
