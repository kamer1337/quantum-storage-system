# Quantum Storage System - API Documentation

## Overview

This document provides comprehensive API documentation for the Quantum Storage System. It covers all public interfaces, data structures, and usage examples for integrating the storage system into your applications.

## Table of Contents

- [Getting Started](#getting-started)
- [Main API - QuantumStorageSystem](#main-api---quantumstoragesystem)
- [Core Components](#core-components)
  - [MLStorageOptimizer](#mlstorageoptimizer)
  - [VirtualStorageManager](#virtualstoragemanager)
  - [AdvancedCompressionSystem](#advancedcompressionsystem)
  - [StorageAnalyticsDashboard](#storageanalyticsdashboard)
  - [CloudStorageIntegration](#cloudstorageintegration)
- [Data Structures](#data-structures)
- [Error Handling](#error-handling)
- [Code Examples](#code-examples)

## Getting Started

### Include Headers

```cpp
#include "quantum_storage_system.h"

// Namespace
using namespace StorageOpt;
```

### Basic Initialization

```cpp
// Create system instance
QuantumStorageSystem system;

// Initialize with storage path and physical limit (5GB)
bool success = system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024);
if (!success) {
    std::cerr << "Failed to initialize storage system" << std::endl;
    return -1;
}

// Start the system (begins background optimization)
system.Start();
```

### Cleanup

```cpp
// Stop background operations
system.Stop();

// Full shutdown
system.Shutdown();
```

## Main API - QuantumStorageSystem

### Class: `QuantumStorageSystem`

Main entry point for all storage operations.

#### Lifecycle Methods

##### `Initialize()`

Initializes the storage system with specified parameters.

```cpp
bool Initialize(const std::string& base_path, size_t physical_limit_bytes);
```

**Parameters**:
- `base_path`: Directory path for storage (will be created if doesn't exist)
- `physical_limit_bytes`: Physical storage limit in bytes

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
// Initialize with 10GB limit
system.Initialize("/mnt/storage", 10ULL * 1024 * 1024 * 1024);
```

---

##### `Start()`

Starts all background optimization threads.

```cpp
void Start();
```

**Effects**:
- Begins ML optimization loop
- Starts tier management
- Initiates analytics collection
- Enables cloud synchronization

---

##### `Stop()`

Stops all background operations gracefully.

```cpp
void Stop();
```

**Effects**:
- Signals all threads to terminate
- Completes pending operations
- Flushes analytics data

---

##### `Shutdown()`

Complete system shutdown and cleanup.

```cpp
void Shutdown();
```

**Effects**:
- Calls Stop() if not already stopped
- Releases all resources
- Closes cloud connections

#### File Operations

##### `CreateFile()`

Creates a new virtual file with specified size.

```cpp
bool CreateFile(const std::string& path, size_t virtual_size);
```

**Parameters**:
- `path`: Virtual file path (e.g., "documents/report.pdf")
- `virtual_size`: Size in bytes of the virtual file

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
// Create a 2GB virtual file
system.CreateFile("data/large_dataset.dat", 2ULL * 1024 * 1024 * 1024);
```

---

##### `WriteFile()`

Writes data to a file. System automatically selects optimal compression.

```cpp
bool WriteFile(const std::string& path, const void* data, size_t size);
```

**Parameters**:
- `path`: Virtual file path
- `data`: Pointer to data buffer
- `size`: Number of bytes to write

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
std::vector<uint8_t> data(1024 * 1024, 42);  // 1MB of data
system.WriteFile("data/file.bin", data.data(), data.size());
```

---

##### `ReadFile()`

Reads data from a file. Automatically decompresses if needed.

```cpp
bool ReadFile(const std::string& path, void* buffer, size_t& size);
```

**Parameters**:
- `path`: Virtual file path
- `buffer`: Buffer to receive data (must be pre-allocated)
- `size`: [in/out] Buffer size on input, bytes read on output

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
size_t buffer_size = 1024 * 1024;  // 1MB buffer
std::vector<uint8_t> buffer(buffer_size);
if (system.ReadFile("data/file.bin", buffer.data(), buffer_size)) {
    // buffer_size now contains actual bytes read
    std::cout << "Read " << buffer_size << " bytes" << std::endl;
}
```

---

##### `DeleteFile()`

Deletes a file from the system.

```cpp
bool DeleteFile(const std::string& path);
```

**Parameters**:
- `path`: Virtual file path to delete

**Returns**: `true` on success, `false` on failure

**Example**:
```cpp
system.DeleteFile("data/old_file.bin");
```

#### System Information

##### `GetVirtualSpaceTotal()`

Returns total virtual space available.

```cpp
size_t GetVirtualSpaceTotal();
```

**Returns**: Virtual space in bytes (typically 2-10x physical limit)

---

##### `GetVirtualSpaceUsed()`

Returns currently used virtual space.

```cpp
size_t GetVirtualSpaceUsed();
```

**Returns**: Used virtual space in bytes

---

##### `GetPhysicalSpaceUsed()`

Returns actual physical storage used.

```cpp
size_t GetPhysicalSpaceUsed();
```

**Returns**: Physical space in bytes

---

##### `GetSpaceMultiplier()`

Returns current space multiplication factor.

```cpp
double GetSpaceMultiplier();
```

**Returns**: Multiplier value (2.0 - 10.0)

**Example**:
```cpp
double multiplier = system.GetSpaceMultiplier();
std::cout << "Current space multiplication: " << multiplier << "x" << std::endl;
// Output: Current space multiplication: 5.2x
```

---

##### `GetStorageEfficiency()`

Returns overall storage efficiency metric.

```cpp
double GetStorageEfficiency();
```

**Returns**: Efficiency value (0.0 - 1.0)
- 1.0 = Maximum efficiency
- 0.5 = Moderate efficiency
- < 0.3 = Low efficiency (optimization recommended)

#### Status and Health

##### `IsHealthy()`

Checks if all subsystems are operating normally.

```cpp
bool IsHealthy();
```

**Returns**: `true` if all systems healthy, `false` otherwise

**Example**:
```cpp
if (!system.IsHealthy()) {
    std::cerr << "System health check failed!" << std::endl;
    std::string status = system.GetSystemStatus();
    std::cerr << status << std::endl;
}
```

---

##### `GetSystemStatus()`

Returns detailed system status string.

```cpp
std::string GetSystemStatus();
```

**Returns**: Human-readable status message

---

##### `GetActiveOptimizations()`

Lists currently active optimization operations.

```cpp
std::vector<std::string> GetActiveOptimizations();
```

**Returns**: Vector of optimization descriptions

**Example**:
```cpp
auto optimizations = system.GetActiveOptimizations();
for (const auto& opt : optimizations) {
    std::cout << "Active: " << opt << std::endl;
}
// Output:
// Active: Compressing cold files (3/10 complete)
// Active: Migrating to cloud (tier: COLD)
// Active: ML model training (epoch 5/10)
```

#### Subsystem Access

##### `GetMLOptimizer()`

Access ML optimization subsystem.

```cpp
MLStorageOptimizer* GetMLOptimizer();
```

**Returns**: Pointer to ML optimizer (never null after Initialize)

---

##### `GetVirtualManager()`

Access virtual storage manager.

```cpp
VirtualStorageManager* GetVirtualManager();
```

---

##### `GetCompressionSystem()`

Access compression subsystem.

```cpp
AdvancedCompressionSystem* GetCompressionSystem();
```

---

##### `GetAnalyticsDashboard()`

Access analytics dashboard.

```cpp
StorageAnalyticsDashboard* GetAnalyticsDashboard();
```

---

##### `GetCloudIntegration()`

Access cloud integration subsystem.

```cpp
CloudStorageIntegration* GetCloudIntegration();
```

## Core Components

### MLStorageOptimizer

Machine learning-based storage optimization.

#### Key Methods

##### `PredictCompressionRatio()`

Predicts compression ratio for a file.

```cpp
double PredictCompressionRatio(const std::string& file_path);
```

**Returns**: Predicted compression ratio (0.0 - 1.0)
- 0.9 = 90% compression (very compressible)
- 0.5 = 50% compression (moderately compressible)
- 0.1 = 10% compression (poorly compressible)

---

##### `SelectOptimalCompression()`

Selects best compression algorithm for file.

```cpp
CompressionAlgorithm SelectOptimalCompression(const FileMetadata& metadata);
```

**Returns**: Optimal compression algorithm configuration

---

##### `CalculateFilePriority()`

Calculates importance priority for a file.

```cpp
double CalculateFilePriority(const FileMetadata& metadata);
```

**Returns**: Priority score (0.0 - 1.0)
- 1.0 = Highest priority (keep in hot tier)
- 0.5 = Medium priority
- 0.0 = Lowest priority (can archive)

---

##### `GetTopPriorityFiles()`

Gets list of highest priority files.

```cpp
std::vector<FileMetadata> GetTopPriorityFiles(int count);
```

**Parameters**:
- `count`: Number of files to return

**Returns**: Vector of file metadata sorted by priority

**Example**:
```cpp
auto ml_optimizer = system.GetMLOptimizer();
auto top_files = ml_optimizer->GetTopPriorityFiles(10);

for (const auto& file : top_files) {
    std::cout << file.path << " - Priority: " << file.ml_priority_score << std::endl;
}
```

---

##### `GetStorageEfficiency()`

Returns ML-calculated storage efficiency.

```cpp
double GetStorageEfficiency();
```

### VirtualStorageManager

Virtual file system and space multiplication management.

#### Key Methods

##### `CreateVirtualFile()`

Creates a virtual file entry.

```cpp
bool CreateVirtualFile(const std::string& virtual_path, size_t size);
```

---

##### `GetVirtualSpaceTotal()`

Returns total virtual space.

```cpp
size_t GetVirtualSpaceTotal();
```

---

##### `GetSpaceMultiplier()`

Returns current quantum space multiplier.

```cpp
double GetSpaceMultiplier();
```

---

##### `SetFileTier()`

Manually set file storage tier.

```cpp
bool SetFileTier(const std::string& virtual_path, StorageTier tier);
```

**Parameters**:
- `virtual_path`: File path
- `tier`: Target tier (HOT, WARM, COLD, FROZEN)

**Example**:
```cpp
auto vm = system.GetVirtualManager();

// Force important file to HOT tier
vm->SetFileTier("critical/config.json", StorageTier::HOT);

// Archive old file to FROZEN tier
vm->SetFileTier("archive/2020/data.zip", StorageTier::FROZEN);
```

---

##### `FileExists()`

Check if virtual file exists.

```cpp
bool FileExists(const std::string& virtual_path);
```

---

##### `GetFileInfo()`

Get detailed file information.

```cpp
VirtualFile GetFileInfo(const std::string& virtual_path);
```

**Returns**: VirtualFile structure with detailed information

---

##### `ListFiles()`

List all files in virtual directory.

```cpp
std::vector<VirtualFile> ListFiles(const std::string& virtual_directory = "");
```

**Parameters**:
- `virtual_directory`: Directory to list (empty = root)

**Returns**: Vector of VirtualFile structures

**Example**:
```cpp
auto vm = system.GetVirtualManager();
auto files = vm->ListFiles("documents");

for (const auto& file : files) {
    std::cout << file.virtual_path 
              << " (" << file.virtual_size << " bytes)" 
              << " Tier: " << static_cast<int>(file.tier)
              << std::endl;
}
```

---

##### `GetCompressionEfficiency()`

Returns compression effectiveness.

```cpp
double GetCompressionEfficiency();
```

---

##### `GetCacheHitRatio()`

Returns cache hit ratio.

```cpp
double GetCacheHitRatio();
```

### AdvancedCompressionSystem

Multi-algorithm compression and deduplication.

#### Key Methods

##### `CompressData()`

Compress data using specified algorithm.

```cpp
CompressionResult CompressData(
    const std::vector<uint8_t>& data, 
    CompressionType type = CompressionType::ML_OPTIMIZED
);
```

**Parameters**:
- `data`: Input data
- `type`: Compression algorithm (default: ML-optimized selection)

**Returns**: CompressionResult with statistics

**Example**:
```cpp
auto compressor = system.GetCompressionSystem();

std::vector<uint8_t> original_data = LoadFile("data.bin");
auto result = compressor->CompressData(original_data, CompressionType::ZSTD_BALANCED);

if (result.success) {
    std::cout << "Original: " << result.original_size << " bytes" << std::endl;
    std::cout << "Compressed: " << result.compressed_size << " bytes" << std::endl;
    std::cout << "Ratio: " << result.compression_ratio << std::endl;
    std::cout << "Time: " << result.compression_time_ms << " ms" << std::endl;
}
```

---

##### `DecompressData()`

Decompress data.

```cpp
bool DecompressData(
    const std::vector<uint8_t>& compressed_data, 
    std::vector<uint8_t>& output, 
    CompressionType type
);
```

**Parameters**:
- `compressed_data`: Compressed input
- `output`: Output buffer (will be resized)
- `type`: Algorithm used for compression

**Returns**: `true` on success

---

##### `SelectOptimalCompression()`

ML-based optimal compression selection.

```cpp
CompressionType SelectOptimalCompression(const std::string& file_path);
CompressionType SelectOptimalCompression(const std::vector<uint8_t>& data);
```

**Returns**: Recommended CompressionType

---

##### `CompressFile()`

Compress file to another file.

```cpp
CompressionResult CompressFile(
    const std::string& input_path, 
    const std::string& output_path, 
    CompressionType type = CompressionType::ML_OPTIMIZED
);
```

---

##### `FindDuplicateFiles()`

Find duplicate files using deduplication.

```cpp
std::vector<std::string> FindDuplicateFiles(const std::string& file_path);
```

**Returns**: Vector of paths to duplicate files

---

##### `FindSimilarFiles()`

Find files with similar content.

```cpp
std::vector<std::string> FindSimilarFiles(
    const std::string& file_path, 
    double similarity_threshold = 0.8
);
```

**Parameters**:
- `file_path`: Reference file
- `similarity_threshold`: Minimum similarity (0.0 - 1.0)

**Returns**: Vector of similar file paths

---

##### `GetSpaceSavedByDeduplication()`

Returns bytes saved through deduplication.

```cpp
size_t GetSpaceSavedByDeduplication();
```

---

##### `BenchmarkAllAlgorithms()`

Benchmark all compression algorithms on sample data.

```cpp
std::vector<CompressionResult> BenchmarkAllAlgorithms(
    const std::vector<uint8_t>& sample_data
);
```

**Returns**: Vector of results for all algorithms

**Example**:
```cpp
auto compressor = system.GetCompressionSystem();
std::vector<uint8_t> sample = LoadSampleData();
auto results = compressor->BenchmarkAllAlgorithms(sample);

std::cout << "Algorithm Benchmark Results:" << std::endl;
for (const auto& result : results) {
    std::cout << "  " << AlgorithmName(result.algorithm_used) 
              << ": ratio=" << result.compression_ratio
              << " time=" << result.compression_time_ms << "ms"
              << std::endl;
}
```

### StorageAnalyticsDashboard

Real-time analytics and predictive insights.

#### Key Methods

##### `RecordMetric()`

Record a performance metric.

```cpp
void RecordMetric(
    const std::string& name, 
    double value, 
    const std::string& unit, 
    const std::string& category = "general"
);
```

**Example**:
```cpp
auto analytics = system.GetAnalyticsDashboard();
analytics->RecordMetric("read_latency", 15.5, "ms", "performance");
analytics->RecordMetric("cache_hits", 127, "count", "cache");
```

---

##### `GetMetricsHistory()`

Get historical metric data.

```cpp
std::vector<PerformanceMetric> GetMetricsHistory(
    const std::string& metric_name,
    std::chrono::system_clock::time_point start_time,
    std::chrono::system_clock::time_point end_time
);
```

---

##### `AnalyzeFileUsagePattern()`

Analyze usage pattern for a file.

```cpp
UsagePattern AnalyzeFileUsagePattern(const std::string& file_path);
```

**Returns**: UsagePattern structure with predictions

**Example**:
```cpp
auto analytics = system.GetAnalyticsDashboard();
auto pattern = analytics->AnalyzeFileUsagePattern("data/report.pdf");

std::cout << "Daily access frequency: " << pattern.daily_access_frequency << std::endl;
std::cout << "Prediction confidence: " << pattern.prediction_confidence << std::endl;

auto next_access = std::chrono::system_clock::to_time_t(pattern.next_predicted_access);
std::cout << "Next predicted access: " << std::ctime(&next_access) << std::endl;
```

---

##### `PredictStorageUsage()`

Predict future storage usage.

```cpp
double PredictStorageUsage(std::chrono::system_clock::time_point target_time);
```

**Returns**: Predicted storage usage in bytes

---

##### `GetInsights()`

Get generated storage insights.

```cpp
std::vector<StorageInsight> GetInsights(const std::string& category = "");
```

**Parameters**:
- `category`: Filter by category (empty = all)

**Returns**: Vector of storage insights

**Example**:
```cpp
auto analytics = system.GetAnalyticsDashboard();
auto insights = analytics->GetInsights();

for (const auto& insight : insights) {
    std::cout << "[" << insight.insight_type << "] " 
              << insight.description << std::endl;
    std::cout << "Impact: " << insight.impact_score 
              << " Confidence: " << insight.confidence_level << std::endl;
    std::cout << "Recommendation: " << insight.recommended_action << std::endl;
    std::cout << std::endl;
}
```

---

##### `GenerateTextReport()`

Generate comprehensive text report.

```cpp
std::string GenerateTextReport(const std::string& report_type = "summary");
```

**Parameters**:
- `report_type`: "summary", "detailed", "performance", "predictions"

**Returns**: Formatted text report

---

##### `GenerateJsonReport()`

Generate JSON-formatted report.

```cpp
std::string GenerateJsonReport();
```

**Returns**: JSON string with all analytics data

### CloudStorageIntegration

Multi-cloud storage integration.

#### Key Methods

##### `AddCloudProvider()`

Add a cloud provider configuration.

```cpp
bool AddCloudProvider(CloudProvider provider, const CloudCredentials& credentials);
```

**Example**:
```cpp
auto cloud = system.GetCloudIntegration();

CloudCredentials azure_creds;
azure_creds.provider = CloudProvider::AZURE_BLOB;
azure_creds.account_name = "mystorageaccount";
azure_creds.access_key = "myaccesskey";
azure_creds.region = "eastus";

cloud->AddCloudProvider(CloudProvider::AZURE_BLOB, azure_creds);
```

---

##### `UploadFile()`

Upload file to cloud.

```cpp
bool UploadFile(
    const std::string& local_path, 
    const std::string& cloud_path,
    CloudProvider provider = CloudProvider::HYBRID_MULTI,
    CloudTier tier = CloudTier::HOT
);
```

**Parameters**:
- `local_path`: Local file path
- `cloud_path`: Cloud destination path
- `provider`: Target provider (HYBRID_MULTI = automatic selection)
- `tier`: Storage tier

---

##### `DownloadFile()`

Download file from cloud.

```cpp
bool DownloadFile(
    const std::string& cloud_path, 
    const std::string& local_path,
    CloudProvider provider
);
```

---

##### `SyncFile()`

Bidirectional file synchronization.

```cpp
bool SyncFile(const std::string& file_path, bool bidirectional = true);
```

---

##### `ListCloudFiles()`

List files in cloud storage.

```cpp
std::vector<CloudFile> ListCloudFiles(
    CloudProvider provider = CloudProvider::HYBRID_MULTI
);
```

---

##### `AutoTierFiles()`

Automatically optimize cloud tier placement.

```cpp
bool AutoTierFiles();
```

---

##### `GetTotalCloudStorage()`

Get total cloud storage used.

```cpp
size_t GetTotalCloudStorage();
```

---

##### `EstimateStorageCost()`

Estimate cloud storage cost.

```cpp
double EstimateStorageCost(size_t bytes, CloudProvider provider, CloudTier tier);
```

**Returns**: Estimated monthly cost in USD

---

##### `GetTotalMonthlyCost()`

Get current total monthly cost.

```cpp
double GetTotalMonthlyCost();
```

## Data Structures

### FileMetadata

```cpp
struct FileMetadata {
    std::string path;
    size_t size;
    std::chrono::system_clock::time_point last_access;
    std::chrono::system_clock::time_point last_modified;
    int access_frequency;
    double compression_ratio;
    bool is_compressed;
    std::string file_type;
    double ml_priority_score;
};
```

### VirtualFile

```cpp
struct VirtualFile {
    std::string virtual_path;
    std::string physical_path;
    size_t virtual_size;
    size_t physical_size;
    StorageTier tier;              // HOT, WARM, COLD, FROZEN
    bool is_cached;
    bool is_compressed;
    std::chrono::system_clock::time_point last_access;
    double priority_score;
    std::string cloud_location;
};
```

### CompressionResult

```cpp
struct CompressionResult {
    bool success;
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double compression_time_ms;
    CompressionType algorithm_used;
    std::string error_message;
};
```

### StorageInsight

```cpp
struct StorageInsight {
    std::string insight_type;
    std::string description;
    double impact_score;            // 0.0 - 1.0
    std::vector<std::string> affected_files;
    std::string recommended_action;
    double confidence_level;        // 0.0 - 1.0
    std::chrono::system_clock::time_point generated_time;
};
```

### UsagePattern

```cpp
struct UsagePattern {
    std::string file_path;
    std::vector<std::chrono::system_clock::time_point> access_times;
    double daily_access_frequency;
    double weekly_access_frequency;
    double monthly_access_frequency;
    std::vector<std::string> access_patterns;
    double prediction_confidence;
    std::chrono::system_clock::time_point next_predicted_access;
};
```

## Enumerations

### CompressionType

```cpp
enum class CompressionType {
    NONE,
    LZ4_FAST,
    LZ4_HIGH,
    ZSTD_FAST,
    ZSTD_BALANCED,
    ZSTD_MAX,
    BROTLI_FAST,
    BROTLI_BALANCED,
    BROTLI_MAX,
    QUANTUM_ADAPTIVE,
    ML_OPTIMIZED            // Automatic ML-based selection
};
```

### StorageTier

```cpp
enum class StorageTier {
    HOT,        // Frequently accessed, local cache
    WARM,       // Occasionally accessed, compressed local
    COLD,       // Rarely accessed, cloud storage
    FROZEN      // Never accessed, deep archive
};
```

### CloudProvider

```cpp
enum class CloudProvider {
    AZURE_BLOB,
    AWS_S3,
    GOOGLE_CLOUD,
    DROPBOX,
    QUANTUM_CLOUD,      // Quantum-enhanced cloud
    HYBRID_MULTI        // Automatic multi-cloud selection
};
```

### CloudTier

```cpp
enum class CloudTier {
    HOT,            // Frequent access
    COOL,           // Infrequent access
    ARCHIVE,        // Long-term storage
    QUANTUM_TIER    // Quantum-optimized
};
```

## Error Handling

The API uses return values for error indication:
- Boolean methods: `true` = success, `false` = failure
- Pointer methods: Non-null = success, null = failure
- Size methods: 0 may indicate error (check with other methods)

For detailed error information:

```cpp
if (!system.WriteFile("path", data, size)) {
    std::string status = system.GetSystemStatus();
    std::cerr << "Write failed: " << status << std::endl;
}
```

## Code Examples

### Example 1: Basic File Operations

```cpp
#include "quantum_storage_system.h"
#include <iostream>
#include <vector>

int main() {
    using namespace StorageOpt;
    
    // Initialize system
    QuantumStorageSystem system;
    if (!system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024)) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    system.Start();
    
    // Create a file
    system.CreateFile("data/report.txt", 1024 * 1024);  // 1MB virtual file
    
    // Write data
    std::string data = "Hello, Quantum Storage!";
    std::vector<uint8_t> buffer(data.begin(), data.end());
    system.WriteFile("data/report.txt", buffer.data(), buffer.size());
    
    // Read data back
    size_t read_size = 1024;
    std::vector<uint8_t> read_buffer(read_size);
    if (system.ReadFile("data/report.txt", read_buffer.data(), read_size)) {
        std::string result(read_buffer.begin(), read_buffer.begin() + read_size);
        std::cout << "Read: " << result << std::endl;
    }
    
    // Check storage stats
    std::cout << "Virtual space: " << system.GetVirtualSpaceTotal() / (1024*1024) << " MB" << std::endl;
    std::cout << "Physical used: " << system.GetPhysicalSpaceUsed() / 1024 << " KB" << std::endl;
    std::cout << "Multiplier: " << system.GetSpaceMultiplier() << "x" << std::endl;
    
    // Cleanup
    system.Stop();
    system.Shutdown();
    
    return 0;
}
```

### Example 2: Compression and Deduplication

```cpp
#include "quantum_storage_system.h"
#include <iostream>

int main() {
    using namespace StorageOpt;
    
    QuantumStorageSystem system;
    system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024);
    system.Start();
    
    auto compressor = system.GetCompressionSystem();
    
    // Load test data
    std::vector<uint8_t> data = LoadLargeDataset();
    
    // Benchmark all algorithms
    auto results = compressor->BenchmarkAllAlgorithms(data);
    
    std::cout << "Compression Benchmark Results:" << std::endl;
    std::cout << "Algorithm\t\tRatio\tTime (ms)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (const auto& result : results) {
        if (result.success) {
            std::cout << AlgorithmName(result.algorithm_used) << "\t"
                      << result.compression_ratio << "\t"
                      << result.compression_time_ms << std::endl;
        }
    }
    
    // Find duplicates
    system.WriteFile("file1.dat", data.data(), data.size());
    system.WriteFile("file2.dat", data.data(), data.size());  // Duplicate
    
    auto duplicates = compressor->FindDuplicateFiles("file1.dat");
    std::cout << "\nFound " << duplicates.size() << " duplicate files" << std::endl;
    
    size_t saved = compressor->GetSpaceSavedByDeduplication();
    std::cout << "Space saved by deduplication: " << saved / 1024 << " KB" << std::endl;
    
    system.Stop();
    return 0;
}
```

### Example 3: ML Optimization and Analytics

```cpp
#include "quantum_storage_system.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    using namespace StorageOpt;
    
    QuantumStorageSystem system;
    system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024);
    system.Start();
    
    auto ml = system.GetMLOptimizer();
    auto analytics = system.GetAnalyticsDashboard();
    
    // Simulate file access patterns
    for (int i = 0; i < 100; ++i) {
        std::string file = "file_" + std::to_string(i) + ".dat";
        system.CreateFile(file, 1024 * 1024);
        
        // Simulate varying access patterns
        int accesses = (i % 10 == 0) ? 50 : (i % 3 == 0) ? 10 : 1;
        for (int j = 0; j < accesses; ++j) {
            size_t size = 1024;
            std::vector<uint8_t> buffer(size);
            system.ReadFile(file, buffer.data(), size);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    // Get top priority files
    std::cout << "Top Priority Files:" << std::endl;
    auto top_files = ml->GetTopPriorityFiles(5);
    for (const auto& file : top_files) {
        std::cout << file.path 
                  << " - Priority: " << file.ml_priority_score
                  << " - Accesses: " << file.access_frequency
                  << std::endl;
    }
    
    // Get analytics insights
    std::cout << "\nStorage Insights:" << std::endl;
    auto insights = analytics->GetInsights();
    for (const auto& insight : insights) {
        std::cout << "[" << insight.insight_type << "] "
                  << insight.description << std::endl;
        std::cout << "  Action: " << insight.recommended_action << std::endl;
    }
    
    // Generate report
    std::string report = analytics->GenerateTextReport("summary");
    std::cout << "\n" << report << std::endl;
    
    system.Stop();
    return 0;
}
```

### Example 4: Cloud Integration

```cpp
#include "quantum_storage_system.h"
#include <iostream>

int main() {
    using namespace StorageOpt;
    
    QuantumStorageSystem system;
    system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024);
    system.Start();
    
    auto cloud = system.GetCloudIntegration();
    
    // Configure cloud provider
    CloudCredentials azure;
    azure.provider = CloudProvider::AZURE_BLOB;
    azure.account_name = "myaccount";
    azure.access_key = "mykey";
    azure.region = "eastus";
    
    if (cloud->AddCloudProvider(CloudProvider::AZURE_BLOB, azure)) {
        std::cout << "Cloud provider configured successfully" << std::endl;
    }
    
    // Upload file to cloud
    system.CreateFile("important.dat", 10 * 1024 * 1024);  // 10MB
    if (cloud->UploadFile("important.dat", "backup/important.dat", 
                          CloudProvider::AZURE_BLOB, CloudTier::HOT)) {
        std::cout << "File uploaded to cloud" << std::endl;
    }
    
    // Auto-tier files based on access patterns
    cloud->AutoTierFiles();
    
    // Check cloud usage
    size_t cloud_bytes = cloud->GetTotalCloudStorage();
    double cost = cloud->GetTotalMonthlyCost();
    
    std::cout << "Cloud storage: " << cloud_bytes / (1024*1024) << " MB" << std::endl;
    std::cout << "Estimated monthly cost: $" << cost << std::endl;
    
    // Get cost optimization suggestions
    auto suggestions = cloud->GetCostOptimizationSuggestions();
    std::cout << "\nCost Optimization Suggestions:" << std::endl;
    for (const auto& [suggestion, savings] : suggestions) {
        std::cout << suggestion << " - Save $" << savings << "/month" << std::endl;
    }
    
    system.Stop();
    return 0;
}
```

### Example 5: Virtual Storage Manager

```cpp
#include "quantum_storage_system.h"
#include <iostream>

int main() {
    using namespace StorageOpt;
    
    QuantumStorageSystem system;
    system.Initialize("./storage", 5ULL * 1024 * 1024 * 1024);
    system.Start();
    
    auto vm = system.GetVirtualManager();
    
    // Create files with different access patterns
    vm->CreateVirtualFile("hot/config.json", 1024);
    vm->CreateVirtualFile("warm/data.csv", 1024 * 1024);
    vm->CreateVirtualFile("cold/archive.zip", 100 * 1024 * 1024);
    
    // Manually set tier for important file
    vm->SetFileTier("hot/config.json", StorageTier::HOT);
    
    // List all files
    auto files = vm->ListFiles();
    std::cout << "Virtual File System:" << std::endl;
    std::cout << "Path\t\t\tSize\t\tTier\tCompressed" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    
    for (const auto& file : files) {
        std::cout << file.virtual_path << "\t"
                  << file.virtual_size / 1024 << " KB\t"
                  << (int)file.tier << "\t"
                  << (file.is_compressed ? "Yes" : "No")
                  << std::endl;
    }
    
    // Space multiplication info
    std::cout << "\nSpace Multiplication:" << std::endl;
    std::cout << "Virtual total: " << vm->GetVirtualSpaceTotal() / (1024*1024) << " MB" << std::endl;
    std::cout << "Physical used: " << vm->GetPhysicalSpaceUsed() / (1024*1024) << " MB" << std::endl;
    std::cout << "Multiplier: " << vm->GetSpaceMultiplier() << "x" << std::endl;
    std::cout << "Cache hit ratio: " << vm->GetCacheHitRatio() << std::endl;
    
    system.Stop();
    return 0;
}
```

## Best Practices

### 1. Initialization

```cpp
// Always check initialization success
if (!system.Initialize(path, limit)) {
    std::cerr << "Failed to initialize: " << system.GetSystemStatus() << std::endl;
    return ERROR;
}
```

### 2. Resource Management

```cpp
// Use RAII pattern
{
    QuantumStorageSystem system;
    system.Initialize(...);
    system.Start();
    
    // Use system
    
    // Automatic cleanup when scope ends
    system.Stop();
}  // Destructor called
```

### 3. Error Checking

```cpp
// Always check operation success
if (!system.WriteFile(path, data, size)) {
    std::cerr << "Write failed" << std::endl;
    // Handle error
}
```

### 4. Performance Optimization

```cpp
// Batch operations when possible
std::vector<std::string> files = GetFilesToUpload();
for (const auto& file : files) {
    cloud->UploadFile(file, "cloud/" + file);
}

// Let the system optimize automatically
system.Start();  // Background optimization begins
```

### 5. Monitoring

```cpp
// Regular health checks
if (!system.IsHealthy()) {
    std::cerr << "System unhealthy: " << system.GetSystemStatus() << std::endl;
    // Take corrective action
}

// Monitor storage efficiency
double efficiency = system.GetStorageEfficiency();
if (efficiency < 0.5) {
    std::cout << "Low efficiency, consider optimization" << std::endl;
}
```

## Thread Safety

All public APIs are thread-safe. The system uses internal locking to ensure safe concurrent access.

```cpp
// Safe to call from multiple threads
std::thread t1([&]() { system.WriteFile("file1", data1, size1); });
std::thread t2([&]() { system.WriteFile("file2", data2, size2); });

t1.join();
t2.join();
```

## Performance Considerations

- **File Size**: Large files (>10MB) are processed in chunks
- **Compression**: ML selects fast algorithms for hot files, high compression for cold files
- **Caching**: Frequently accessed files kept in uncompressed cache
- **Background Threads**: Optimization happens asynchronously without blocking API calls

## Version Compatibility

Current API Version: 1.0.0

API stability guarantees:
- Patch versions (1.0.x): No breaking changes
- Minor versions (1.x.0): Backward compatible additions
- Major versions (x.0.0): May include breaking changes

## See Also

- [Architecture Documentation](ARCHITECTURE.md) - Detailed system architecture
- [Installation Guide](INSTALLATION.md) - Installation and build instructions
- [README](../README.md) - Project overview and quick start

## Support

For API questions and issues:
- GitHub Issues: [quantum-storage-system/issues](https://github.com/kamer1337/quantum-storage-system/issues)
- Documentation: [docs/](../docs/)
- Examples: [examples/](../examples/)
