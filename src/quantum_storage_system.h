#pragma once

#include "core/ml_storage_optimizer.h"
#include "core/virtual_storage_manager.h"
#include "core/advanced_compression_system.h"
#include "core/usb_device_driver.h"
#include "core/encryption_manager.h"
#include "core/batch_operation_manager.h"
#include "core/health_monitor.h"
#include "analytics/storage_analytics_dashboard.h"
#include "cloud/cloud_storage_integration.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace StorageOpt {

class QuantumStorageSystem {
private:
    std::unique_ptr<MLStorageOptimizer> ml_optimizer_;
    std::unique_ptr<VirtualStorageManager> virtual_manager_;
    std::unique_ptr<AdvancedCompressionSystem> compression_system_;
    std::unique_ptr<USBDeviceDriver> usb_driver_;
    std::unique_ptr<StorageAnalyticsDashboard> analytics_dashboard_;
    std::unique_ptr<CloudStorageIntegration> cloud_integration_;
    std::unique_ptr<EncryptionManager> encryption_manager_;
    std::unique_ptr<BatchOperationManager> batch_operation_manager_;
    std::unique_ptr<HealthMonitor> health_monitor_;
    
    bool initialized_;
    std::string base_path_;
    size_t physical_limit_;

public:
    QuantumStorageSystem();
    ~QuantumStorageSystem();
    
    // System lifecycle
    bool Initialize(const std::string& base_path, size_t physical_limit_bytes);
    void Start();
    void Stop();
    void Shutdown();
    
    // Core functionality
    bool CreateFile(const std::string& path, size_t virtual_size);
    bool WriteFile(const std::string& path, const void* data, size_t size);
    bool ReadFile(const std::string& path, void* buffer, size_t& size);
    bool DeleteFile(const std::string& path);
    
    // System information
    size_t GetVirtualSpaceTotal();
    size_t GetVirtualSpaceUsed();
    size_t GetPhysicalSpaceUsed();
    double GetSpaceMultiplier();
    double GetStorageEfficiency();
    
    // Status and health
    bool IsHealthy();
    std::string GetSystemStatus();
    std::vector<std::string> GetActiveOptimizations();
    
    // Access to subsystems
    MLStorageOptimizer* GetMLOptimizer() { return ml_optimizer_.get(); }
    VirtualStorageManager* GetVirtualManager() { return virtual_manager_.get(); }
    AdvancedCompressionSystem* GetCompressionSystem() { return compression_system_.get(); }
    USBDeviceDriver* GetUSBDriver() { return usb_driver_.get(); }
    StorageAnalyticsDashboard* GetAnalyticsDashboard() { return analytics_dashboard_.get(); }
    CloudStorageIntegration* GetCloudIntegration() { return cloud_integration_.get(); }
    EncryptionManager* GetEncryptionManager() { return encryption_manager_.get(); }
    BatchOperationManager* GetBatchOperationManager() { return batch_operation_manager_.get(); }
    HealthMonitor* GetHealthMonitor() { return health_monitor_.get(); }
};

} // namespace StorageOpt