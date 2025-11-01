#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <queue>

namespace StorageOpt {

// Forward declaration
class QuantumStorageSystem;

enum class HealthStatus {
    HEALTHY,
    WARNING,
    CRITICAL,
    UNKNOWN
};

enum class AlertSeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

struct HealthMetric {
    std::string name;
    double value;
    double threshold_warning;
    double threshold_critical;
    HealthStatus status;
    std::string unit;
    std::chrono::system_clock::time_point last_updated;
};

struct SystemAlert {
    AlertSeverity severity;
    std::string message;
    std::string component;
    std::chrono::system_clock::time_point timestamp;
    bool acknowledged;
    std::string details;
};

using AlertCallback = std::function<void(const SystemAlert&)>;

class HealthMonitor {
private:
    QuantumStorageSystem* storage_system_;
    
    // Health metrics
    std::vector<HealthMetric> health_metrics_;
    std::mutex metrics_mutex_;
    
    // Alert system
    std::queue<SystemAlert> alert_queue_;
    std::vector<SystemAlert> alert_history_;
    std::mutex alerts_mutex_;
    AlertCallback alert_callback_;
    
    // Monitoring thread
    std::atomic<bool> running_;
    std::thread monitoring_thread_;
    std::chrono::seconds check_interval_;
    
    // Health thresholds
    struct Thresholds {
        double cpu_usage_warning = 70.0;
        double cpu_usage_critical = 90.0;
        double memory_usage_warning = 80.0;
        double memory_usage_critical = 95.0;
        double disk_usage_warning = 85.0;
        double disk_usage_critical = 95.0;
        double compression_ratio_warning = 1.5;
        double space_multiplier_warning = 1.5;
        double cache_hit_ratio_warning = 0.5;
    } thresholds_;

public:
    HealthMonitor(QuantumStorageSystem* system);
    ~HealthMonitor();
    
    // Initialization
    bool Initialize(std::chrono::seconds check_interval = std::chrono::seconds(30));
    void Shutdown();
    
    // Health monitoring
    HealthStatus GetOverallHealth();
    std::vector<HealthMetric> GetAllMetrics();
    HealthMetric GetMetric(const std::string& name);
    void UpdateMetric(const std::string& name, double value);
    
    // Alert management
    void RaiseAlert(AlertSeverity severity, const std::string& component, const std::string& message, const std::string& details = "");
    std::vector<SystemAlert> GetActiveAlerts();
    std::vector<SystemAlert> GetAlertHistory(size_t max_count = 100);
    bool AcknowledgeAlert(size_t alert_index);
    void ClearAlertHistory();
    
    // Alert callbacks
    void SetAlertCallback(AlertCallback callback);
    void ClearAlertCallback();
    
    // Health checks
    bool CheckSystemHealth();
    bool CheckStorageHealth();
    bool CheckCompressionHealth();
    bool CheckMLOptimizerHealth();
    bool CheckCloudIntegrationHealth();
    
    // Threshold configuration
    void SetThreshold(const std::string& metric_name, double warning, double critical);
    void ResetThresholds();
    
    // Statistics
    size_t GetTotalAlerts();
    size_t GetAlertsOfSeverity(AlertSeverity severity);
    double GetUptimeSeconds();
    
private:
    // Monitoring loop
    void MonitoringLoop();
    
    // Health check implementations
    void CheckCPUUsage();
    void CheckMemoryUsage();
    void CheckDiskUsage();
    void CheckCompressionRatio();
    void CheckSpaceMultiplier();
    void CheckCacheHitRatio();
    void CheckSystemTemperature();
    
    // Helper functions
    HealthStatus DetermineHealthStatus(double value, double warning_threshold, double critical_threshold, bool higher_is_worse = true);
    void NotifyAlert(const SystemAlert& alert);
    std::string HealthStatusToString(HealthStatus status);
    std::string AlertSeverityToString(AlertSeverity severity);
    
    // System metrics collection
    double GetCPUUsage();
    double GetMemoryUsage();
    double GetDiskUsage();
    
    std::chrono::system_clock::time_point start_time_;
};

} // namespace StorageOpt
