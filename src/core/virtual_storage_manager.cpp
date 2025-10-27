#include "virtual_storage_manager.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace StorageOpt {

VirtualStorageManager::VirtualStorageManager() 
    : running_(false), max_cache_size_(1024 * 1024 * 1024) { // 1GB default cache
    
    // Initialize quantum multiplier parameters
    quantum_multiplier_.base_factor = 2.0;
    quantum_multiplier_.entanglement_boost = 0.5;
    quantum_multiplier_.compression_factor = 0.3;
    quantum_multiplier_.cloud_factor = 1.5;
    quantum_multiplier_.ml_optimization_factor = 0.4;
}

VirtualStorageManager::~VirtualStorageManager() {
    Stop();
}

bool VirtualStorageManager::Initialize(const std::string& base_path, size_t physical_limit) {
    try {
        base_path_ = base_path;
        cache_path_ = base_path + "/cache";
        cloud_path_ = base_path + "/cloud_sync";
        
        // Create directory structure
        std::filesystem::create_directories(base_path_);
        std::filesystem::create_directories(cache_path_);
        std::filesystem::create_directories(cloud_path_);
        std::filesystem::create_directories(base_path_ + "/hot");
        std::filesystem::create_directories(base_path_ + "/warm");
        std::filesystem::create_directories(base_path_ + "/cold");
        std::filesystem::create_directories(base_path_ + "/frozen");
        
        // Initialize storage quota
        quota_.physical_limit = physical_limit;
        quota_.current_physical = 0;
        quota_.current_virtual = 0;
        
        // Calculate initial quantum multiplier
        RecalculateQuantumMultiplier();
        quota_.virtual_limit = static_cast<size_t>(physical_limit * quota_.multiplier_factor);
        
        std::cout << "Virtual Storage Manager initialized:" << std::endl;
        std::cout << "  Physical limit: " << physical_limit / 1024 / 1024 << " MB" << std::endl;
        std::cout << "  Virtual limit: " << quota_.virtual_limit / 1024 / 1024 << " MB" << std::endl;
        std::cout << "  Quantum multiplier: " << quota_.multiplier_factor << "x" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Virtual Storage Manager: " << e.what() << std::endl;
        return false;
    }
}

void VirtualStorageManager::Start() {
    running_ = true;
    tier_management_thread_ = std::thread(&VirtualStorageManager::TierManagementLoop, this);
    cache_management_thread_ = std::thread(&VirtualStorageManager::CacheManagementLoop, this);
    quantum_multiplier_thread_ = std::thread(&VirtualStorageManager::QuantumMultiplierLoop, this);
    
    std::cout << "Virtual Storage Manager started." << std::endl;
}

void VirtualStorageManager::Stop() {
    running_ = false;
    
    if (tier_management_thread_.joinable()) {
        tier_management_thread_.join();
    }
    if (cache_management_thread_.joinable()) {
        cache_management_thread_.join();
    }
    if (quantum_multiplier_thread_.joinable()) {
        quantum_multiplier_thread_.join();
    }
    
    std::cout << "Virtual Storage Manager stopped." << std::endl;
}

bool VirtualStorageManager::CreateVirtualFile(const std::string& virtual_path, size_t size) {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    // Check if we have virtual space
    if (quota_.current_virtual + size > quota_.virtual_limit) {
        // Try to expand quantum space
        if (!ExpandVirtualSpace(1.1)) {
            std::cerr << "Insufficient virtual space for file: " << virtual_path << std::endl;
            return false;
        }
    }
    
    // Create virtual file entry
    VirtualFile vfile;
    vfile.virtual_path = virtual_path;
    vfile.virtual_size = size;
    vfile.physical_size = 0; // Will be set when data is written
    vfile.tier = StorageTier::HOT; // Start in hot tier
    vfile.is_cached = false;
    vfile.is_compressed = false;
    vfile.last_access = std::chrono::system_clock::now();
    vfile.priority_score = 1.0; // New files start with high priority
    
    // Determine physical path based on tier
    vfile.physical_path = GetPhysicalPath(virtual_path, vfile.tier);
    
    virtual_files_[virtual_path] = vfile;
    quota_.current_virtual += size;
    
    std::cout << "Created virtual file: " << virtual_path 
              << " (Virtual size: " << size << " bytes)" << std::endl;
    
    return true;
}

bool VirtualStorageManager::WriteVirtualFile(const std::string& virtual_path, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    auto it = virtual_files_.find(virtual_path);
    if (it == virtual_files_.end()) {
        std::cerr << "Virtual file not found: " << virtual_path << std::endl;
        return false;
    }
    
    VirtualFile& vfile = it->second;
    
    // Check physical space availability
    if (!HasPhysicalSpace(size)) {
        // Try to free space by tier management
        OptimizeAllTiers();
        if (!HasPhysicalSpace(size)) {
            std::cerr << "Insufficient physical space for writing file: " << virtual_path << std::endl;
            return false;
        }
    }
    
    try {
        // Ensure directory exists
        EnsureDirectoryExists(std::filesystem::path(vfile.physical_path).parent_path().string());
        
        // Write data to physical file
        std::ofstream file(vfile.physical_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open physical file for writing: " << vfile.physical_path << std::endl;
            return false;
        }
        
        file.write(static_cast<const char*>(data), size);
        file.close();
        
        // Update file metadata
        vfile.physical_size = size;
        vfile.last_access = std::chrono::system_clock::now();
        quota_.current_physical += size;
        
        // Apply quantum optimization
        double quantum_factor = CalculateFileQuantumFactor(vfile);
        vfile.priority_score *= quantum_factor;
        
        std::cout << "Written virtual file: " << virtual_path 
                  << " (Physical size: " << size << " bytes)" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing virtual file: " << e.what() << std::endl;
        return false;
    }
}

bool VirtualStorageManager::ReadVirtualFile(const std::string& virtual_path, void* buffer, size_t& size) {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    auto it = virtual_files_.find(virtual_path);
    if (it == virtual_files_.end()) {
        std::cerr << "Virtual file not found: " << virtual_path << std::endl;
        return false;
    }
    
    VirtualFile& vfile = it->second;
    
    // Update access time
    vfile.last_access = std::chrono::system_clock::now();
    
    // Check if file needs to be retrieved from cloud or cache
    if (vfile.tier == StorageTier::FROZEN && !vfile.cloud_location.empty()) {
        if (!DownloadFromCloud(virtual_path)) {
            std::cerr << "Failed to download file from cloud: " << virtual_path << std::endl;
            return false;
        }
    }
    
    try {
        std::ifstream file(vfile.physical_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open physical file for reading: " << vfile.physical_path << std::endl;
            return false;
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (size < file_size) {
            std::cerr << "Buffer too small for file: " << virtual_path << std::endl;
            size = file_size;
            return false;
        }
        
        file.read(static_cast<char*>(buffer), file_size);
        size = file_size;
        
        // Consider caching if file is accessed frequently
        if (ShouldCacheFile(vfile)) {
            CacheFile(virtual_path);
        }
        
        // Update priority based on access pattern
        vfile.priority_score = std::min(vfile.priority_score + 0.1, 2.0);
        
        std::cout << "Read virtual file: " << virtual_path 
                  << " (" << size << " bytes)" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error reading virtual file: " << e.what() << std::endl;
        return false;
    }
}

bool VirtualStorageManager::DeleteVirtualFile(const std::string& virtual_path) {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    auto it = virtual_files_.find(virtual_path);
    if (it == virtual_files_.end()) {
        std::cerr << "Virtual file not found: " << virtual_path << std::endl;
        return false;
    }
    
    VirtualFile& vfile = it->second;
    
    try {
        // Delete physical file if it exists
        if (std::filesystem::exists(vfile.physical_path)) {
            std::filesystem::remove(vfile.physical_path);
        }
        
        // Remove from cloud if present
        if (!vfile.cloud_location.empty()) {
            // Note: Cloud deletion would be handled by cloud integration
            std::cout << "Virtual file had cloud location: " << vfile.cloud_location << std::endl;
        }
        
        // Update quota
        quota_.current_physical -= vfile.virtual_size;
        quota_.current_virtual -= vfile.virtual_size;
        
        // Remove from virtual files map
        virtual_files_.erase(it);
        
        std::cout << "Deleted virtual file: " << virtual_path << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error deleting virtual file: " << e.what() << std::endl;
        return false;
    }
}

size_t VirtualStorageManager::GetVirtualSpaceTotal() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    return quota_.virtual_limit;
}

size_t VirtualStorageManager::GetVirtualSpaceUsed() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    return quota_.current_virtual;
}

size_t VirtualStorageManager::GetVirtualSpaceFree() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    return quota_.virtual_limit - quota_.current_virtual;
}

size_t VirtualStorageManager::GetPhysicalSpaceUsed() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    return quota_.current_physical;
}

double VirtualStorageManager::GetSpaceMultiplier() {
    return quota_.multiplier_factor;
}

void VirtualStorageManager::RecalculateQuantumMultiplier() {
    double base = quantum_multiplier_.base_factor;
    
    // Add compression efficiency boost
    double compression_boost = GetCompressionEfficiency() * quantum_multiplier_.compression_factor;
    
    // Add cloud integration boost
    double cloud_boost = (GetCloudStorageUsed() > 0) ? quantum_multiplier_.cloud_factor : 0.0;
    
    // Add ML optimization boost
    double ml_boost = quantum_multiplier_.ml_optimization_factor;
    
    // Calculate entanglement effect across all files
    double entanglement_effect = 0.0;
    {
        std::lock_guard<std::mutex> lock(files_mutex_);
        for (const auto& pair : virtual_files_) {
            entanglement_effect += CalculateQuantumEntanglement(pair.second);
        }
        entanglement_effect /= std::max(static_cast<double>(virtual_files_.size()), 1.0);
    }
    
    entanglement_effect *= quantum_multiplier_.entanglement_boost;
    
    // Apply quantum superposition effect
    quota_.multiplier_factor = ApplyQuantumSuperposition(
        base + compression_boost + cloud_boost + ml_boost + entanglement_effect
    );
    
    // Ensure reasonable bounds
    quota_.multiplier_factor = std::clamp(quota_.multiplier_factor, 1.5, 10.0);
    
    std::cout << "Quantum multiplier recalculated: " << quota_.multiplier_factor << "x" << std::endl;
}

double VirtualStorageManager::CalculateFileQuantumFactor(const VirtualFile& file) {
    // Calculate quantum entanglement with other files
    double entanglement = CalculateQuantumEntanglement(file);
    
    // Factor in file access patterns
    auto now = std::chrono::system_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::hours>(now - file.last_access).count();
    double access_factor = std::exp(-time_diff / 24.0); // Decay over 24 hours
    
    // Factor in compression ratio
    double compression_factor = 1.0;
    if (file.is_compressed && file.physical_size > 0) {
        compression_factor = static_cast<double>(file.virtual_size) / static_cast<double>(file.physical_size);
    }
    
    // Combine factors with quantum effects
    double quantum_factor = 1.0 + (entanglement * 0.2) + (access_factor * 0.1) + (compression_factor * 0.05);
    
    return std::clamp(quantum_factor, 1.0, 2.0);
}

bool VirtualStorageManager::ExpandVirtualSpace(double additional_factor) {
    double new_multiplier = quota_.multiplier_factor * additional_factor;
    
    // Check if expansion is physically possible
    if (new_multiplier > 10.0) {
        std::cout << "Cannot expand beyond 10x quantum limit." << std::endl;
        return false;
    }
    
    // Apply quantum expansion
    quota_.multiplier_factor = new_multiplier;
    quota_.virtual_limit = static_cast<size_t>(quota_.physical_limit * quota_.multiplier_factor);
    
    std::cout << "Expanded virtual space to " << quota_.virtual_limit / 1024 / 1024 
              << " MB (multiplier: " << quota_.multiplier_factor << "x)" << std::endl;
    
    return true;
}

StorageTier VirtualStorageManager::PredictOptimalTier(const VirtualFile& file) {
    auto now = std::chrono::system_clock::now();
    auto hours_since_access = std::chrono::duration_cast<std::chrono::hours>(
        now - file.last_access).count();
    
    // Quantum-enhanced tier prediction
    double quantum_factor = CalculateFileQuantumFactor(file);
    double adjusted_hours = hours_since_access / quantum_factor;
    
    if (adjusted_hours < 1) {
        return StorageTier::HOT;
    } else if (adjusted_hours < 24) {
        return StorageTier::WARM;
    } else if (adjusted_hours < 168) { // 1 week
        return StorageTier::COLD;
    } else {
        return StorageTier::FROZEN;
    }
}

std::string VirtualStorageManager::GetPhysicalPath(const std::string& virtual_path, StorageTier tier) {
    std::string tier_dir;
    switch (tier) {
        case StorageTier::HOT:    tier_dir = "hot"; break;
        case StorageTier::WARM:   tier_dir = "warm"; break;
        case StorageTier::COLD:   tier_dir = "cold"; break;
        case StorageTier::FROZEN: tier_dir = "frozen"; break;
    }
    
    // Create a safe filename from virtual path
    std::string safe_name = virtual_path;
    std::replace(safe_name.begin(), safe_name.end(), '/', '_');
    std::replace(safe_name.begin(), safe_name.end(), '\\', '_');
    std::replace(safe_name.begin(), safe_name.end(), ':', '_');
    
    return base_path_ + "/" + tier_dir + "/" + safe_name;
}

bool VirtualStorageManager::HasPhysicalSpace(size_t required_size) {
    return (quota_.current_physical + required_size) <= quota_.physical_limit;
}

double VirtualStorageManager::CalculateQuantumEntanglement(const VirtualFile& file) {
    // Simulate quantum entanglement based on file relationships
    double entanglement = 0.0;
    
    // Files in same directory are more entangled
    std::filesystem::path file_path(file.virtual_path);
    std::string directory = file_path.parent_path().string();
    
    std::lock_guard<std::mutex> lock(files_mutex_);
    int related_files = 0;
    for (const auto& pair : virtual_files_) {
        if (pair.first != file.virtual_path) {
            std::filesystem::path other_path(pair.first);
            if (other_path.parent_path().string() == directory) {
                related_files++;
            }
        }
    }
    
    // Entanglement increases with number of related files
    entanglement = std::tanh(related_files / 10.0) * 0.5;
    
    // File extension correlation
    std::string extension = file_path.extension().string();
    for (const auto& pair : virtual_files_) {
        if (pair.first != file.virtual_path) {
            std::filesystem::path other_path(pair.first);
            if (other_path.extension().string() == extension) {
                entanglement += 0.1;
            }
        }
    }
    
    return std::clamp(entanglement, 0.0, 1.0);
}

double VirtualStorageManager::ApplyQuantumSuperposition(double base_multiplier) {
    // Simulate quantum superposition effect on space multiplication
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(0.0, 0.1); // Small random variation
    
    double quantum_noise = dis(gen);
    double superposed_multiplier = base_multiplier * (1.0 + quantum_noise);
    
    // Apply quantum interference patterns
    double interference = std::sin(base_multiplier * M_PI) * 0.05;
    superposed_multiplier += interference;
    
    return superposed_multiplier;
}

void VirtualStorageManager::TierManagementLoop() {
    while (running_) {
        try {
            OptimizeAllTiers();
            std::this_thread::sleep_for(std::chrono::minutes(10)); // Run every 10 minutes
        } catch (const std::exception& e) {
            std::cerr << "Error in tier management loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

void VirtualStorageManager::CacheManagementLoop() {
    while (running_) {
        try {
            OptimizeCache();
            std::this_thread::sleep_for(std::chrono::minutes(5)); // Run every 5 minutes
        } catch (const std::exception& e) {
            std::cerr << "Error in cache management loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

void VirtualStorageManager::QuantumMultiplierLoop() {
    while (running_) {
        try {
            RecalculateQuantumMultiplier();
            RecalculateVirtualLimits();
            std::this_thread::sleep_for(std::chrono::minutes(15)); // Run every 15 minutes
        } catch (const std::exception& e) {
            std::cerr << "Error in quantum multiplier loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(2));
        }
    }
}

void VirtualStorageManager::OptimizeAllTiers() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    for (auto& pair : virtual_files_) {
        VirtualFile& file = pair.second;
        StorageTier optimal_tier = PredictOptimalTier(file);
        
        if (optimal_tier != file.tier) {
            // Move file to optimal tier
            std::string new_physical_path = GetPhysicalPath(file.virtual_path, optimal_tier);
            
            try {
                if (std::filesystem::exists(file.physical_path)) {
                    EnsureDirectoryExists(std::filesystem::path(new_physical_path).parent_path().string());
                    std::filesystem::rename(file.physical_path, new_physical_path);
                    file.physical_path = new_physical_path;
                    file.tier = optimal_tier;
                    
                    std::cout << "Moved file " << file.virtual_path 
                              << " to tier " << static_cast<int>(optimal_tier) << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error moving file to optimal tier: " << e.what() << std::endl;
            }
        }
    }
}

void VirtualStorageManager::RecalculateVirtualLimits() {
    size_t new_virtual_limit = static_cast<size_t>(quota_.physical_limit * quota_.multiplier_factor);
    
    if (new_virtual_limit != quota_.virtual_limit) {
        quota_.virtual_limit = new_virtual_limit;
        std::cout << "Virtual space limit updated to " << quota_.virtual_limit / 1024 / 1024 << " MB" << std::endl;
    }
}

bool VirtualStorageManager::EnsureDirectoryExists(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory " << path << ": " << e.what() << std::endl;
        return false;
    }
}

double VirtualStorageManager::GetCompressionEfficiency() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    size_t total_virtual = 0;
    size_t total_physical = 0;
    
    for (const auto& pair : virtual_files_) {
        const VirtualFile& file = pair.second;
        if (file.is_compressed) {
            total_virtual += file.virtual_size;
            total_physical += file.physical_size;
        }
    }
    
    if (total_virtual == 0) return 0.0;
    
    return 1.0 - (static_cast<double>(total_physical) / static_cast<double>(total_virtual));
}

size_t VirtualStorageManager::GetCloudStorageUsed() {
    std::lock_guard<std::mutex> lock(files_mutex_);
    
    size_t cloud_usage = 0;
    for (const auto& pair : virtual_files_) {
        if (!pair.second.cloud_location.empty()) {
            cloud_usage += pair.second.virtual_size;
        }
    }
    
    return cloud_usage;
}

bool VirtualStorageManager::ShouldCacheFile(const VirtualFile& file) {
    // Cache files that are accessed frequently and small enough
    return file.priority_score > 1.5 && file.physical_size < (max_cache_size_ / 10);
}

void VirtualStorageManager::OptimizeCache() {
    // Simple cache optimization - implement LRU eviction if needed
    std::lock_guard<std::mutex> lock(cache_mutex_);
    
    // Calculate current cache usage
    size_t cache_usage = 0;
    for (const auto& pair : virtual_files_) {
        if (pair.second.is_cached) {
            cache_usage += pair.second.physical_size;
        }
    }
    
    // If cache is over limit, evict least recently used files
    if (cache_usage > max_cache_size_) {
        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> cached_files;
        
        for (const auto& pair : virtual_files_) {
            if (pair.second.is_cached) {
                cached_files.push_back({pair.first, pair.second.last_access});
            }
        }
        
        // Sort by last access time (oldest first)
        std::sort(cached_files.begin(), cached_files.end(),
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });
        
        // Evict oldest files until we're under the limit
        for (const auto& file_pair : cached_files) {
            if (cache_usage <= max_cache_size_) break;
            
            EvictFromCache(file_pair.first);
            cache_usage -= virtual_files_[file_pair.first].physical_size;
        }
    }
}

bool VirtualStorageManager::CacheFile(const std::string& virtual_path) {
    // Implementation would copy file to cache location
    std::cout << "Caching file: " << virtual_path << std::endl;
    return true;
}

bool VirtualStorageManager::EvictFromCache(const std::string& virtual_path) {
    // Implementation would remove file from cache
    std::cout << "Evicting from cache: " << virtual_path << std::endl;
    return true;
}

// Placeholder implementations for cloud integration
bool VirtualStorageManager::UploadToCloud(const std::string& virtual_path) {
    std::cout << "Uploading to cloud: " << virtual_path << std::endl;
    return true;
}

bool VirtualStorageManager::DownloadFromCloud(const std::string& virtual_path) {
    std::cout << "Downloading from cloud: " << virtual_path << std::endl;
    return true;
}

bool VirtualStorageManager::IsInCloud(const std::string& virtual_path) {
    auto it = virtual_files_.find(virtual_path);
    return it != virtual_files_.end() && !it->second.cloud_location.empty();
}

} // namespace StorageOpt