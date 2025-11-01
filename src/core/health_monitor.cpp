#include "health_monitor.h"
#include "../quantum_storage_system.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>

namespace StorageOpt {

HealthMonitor::HealthMonitor(QuantumStorageSystem* system)
    : storage_system_(system)
    , running_(false)
    , check_interval_(30)
    , start_time_(std::chrono::system_clock::now()) {
}

HealthMonitor::~HealthMonitor() {
    Shutdown();
}

bool HealthMonitor::Initialize(std::chrono::seconds check_interval) {
    check_interval_ = check_interval;
    
    // Initialize health metrics
    health_metrics_.push_back({"cpu_usage", 0.0, thresholds_.cpu_usage_warning, thresholds_.cpu_usage_critical, HealthStatus::UNKNOWN, "%", std::chrono::system_clock::now()});
    health_metrics_.push_back({"memory_usage", 0.0, thresholds_.memory_usage_warning, thresholds_.memory_usage_critical, HealthStatus::UNKNOWN, "%", std::chrono::system_clock::now()});
    health_metrics_.push_back({"disk_usage", 0.0, thresholds_.disk_usage_warning, thresholds_.disk_usage_critical, HealthStatus::UNKNOWN, "%", std::chrono::system_clock::now()});
    health_metrics_.push_back({"compression_ratio", 0.0, thresholds_.compression_ratio_warning, 0.0, HealthStatus::UNKNOWN, "x", std::chrono::system_clock::now()});
    health_metrics_.push_back({"space_multiplier", 0.0, thresholds_.space_multiplier_warning, 0.0, HealthStatus::UNKNOWN, "x", std::chrono::system_clock::now()});
    health_metrics_.push_back({"cache_hit_ratio", 0.0, thresholds_.cache_hit_ratio_warning, 0.0, HealthStatus::UNKNOWN, "%", std::chrono::system_clock::now()});
    
    // Start monitoring thread
    running_ = true;
    monitoring_thread_ = std::thread(&HealthMonitor::MonitoringLoop, this);
    
    return true;
}

void HealthMonitor::Shutdown() {
    running_ = false;
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
}

HealthStatus HealthMonitor::GetOverallHealth() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    HealthStatus overall = HealthStatus::HEALTHY;
    
    for (const auto& metric : health_metrics_) {
        if (metric.status == HealthStatus::CRITICAL) {
            return HealthStatus::CRITICAL;
        } else if (metric.status == HealthStatus::WARNING && overall == HealthStatus::HEALTHY) {
            overall = HealthStatus::WARNING;
        }
    }
    
    return overall;
}

std::vector<HealthMetric> HealthMonitor::GetAllMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return health_metrics_;
}

HealthMetric HealthMonitor::GetMetric(const std::string& name) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    for (const auto& metric : health_metrics_) {
        if (metric.name == name) {
            return metric;
        }
    }
    
    return {"", 0.0, 0.0, 0.0, HealthStatus::UNKNOWN, "", std::chrono::system_clock::now()};
}

void HealthMonitor::UpdateMetric(const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    for (auto& metric : health_metrics_) {
        if (metric.name == name) {
            metric.value = value;
            metric.last_updated = std::chrono::system_clock::now();
            
            // Determine health status based on thresholds
            if (metric.threshold_critical > 0.0) {
                metric.status = DetermineHealthStatus(value, metric.threshold_warning, metric.threshold_critical);
            } else {
                // For metrics where higher is better (like compression ratio)
                metric.status = DetermineHealthStatus(value, metric.threshold_warning, 0.0, false);
            }
            
            // Raise alert if status is warning or critical
            if (metric.status == HealthStatus::WARNING || metric.status == HealthStatus::CRITICAL) {
                AlertSeverity severity = (metric.status == HealthStatus::CRITICAL) ? AlertSeverity::CRITICAL : AlertSeverity::WARNING;
                std::stringstream ss;
                ss << metric.name << " is " << value << metric.unit << " (threshold: " << metric.threshold_warning << metric.unit << ")";
                RaiseAlert(severity, "HealthMonitor", ss.str());
            }
            
            break;
        }
    }
}

void HealthMonitor::RaiseAlert(AlertSeverity severity, const std::string& component, const std::string& message, const std::string& details) {
    SystemAlert alert;
    alert.severity = severity;
    alert.component = component;
    alert.message = message;
    alert.details = details;
    alert.timestamp = std::chrono::system_clock::now();
    alert.acknowledged = false;
    
    {
        std::lock_guard<std::mutex> lock(alerts_mutex_);
        alert_queue_.push(alert);
        alert_history_.push_back(alert);
    }
    
    NotifyAlert(alert);
}

std::vector<SystemAlert> HealthMonitor::GetActiveAlerts() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    std::vector<SystemAlert> active_alerts;
    std::queue<SystemAlert> temp_queue = alert_queue_;
    
    while (!temp_queue.empty()) {
        active_alerts.push_back(temp_queue.front());
        temp_queue.pop();
    }
    
    return active_alerts;
}

std::vector<SystemAlert> HealthMonitor::GetAlertHistory(size_t max_count) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    size_t start_idx = alert_history_.size() > max_count ? alert_history_.size() - max_count : 0;
    return std::vector<SystemAlert>(alert_history_.begin() + start_idx, alert_history_.end());
}

bool HealthMonitor::AcknowledgeAlert(size_t alert_index) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    if (alert_index < alert_history_.size()) {
        alert_history_[alert_index].acknowledged = true;
        return true;
    }
    
    return false;
}

void HealthMonitor::ClearAlertHistory() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_history_.clear();
    while (!alert_queue_.empty()) {
        alert_queue_.pop();
    }
}

void HealthMonitor::SetAlertCallback(AlertCallback callback) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_callback_ = callback;
}

void HealthMonitor::ClearAlertCallback() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_callback_ = nullptr;
}

bool HealthMonitor::CheckSystemHealth() {
    CheckCPUUsage();
    CheckMemoryUsage();
    CheckDiskUsage();
    CheckSystemTemperature();
    return GetOverallHealth() != HealthStatus::CRITICAL;
}

bool HealthMonitor::CheckStorageHealth() {
    CheckDiskUsage();
    CheckSpaceMultiplier();
    return true;
}

bool HealthMonitor::CheckCompressionHealth() {
    CheckCompressionRatio();
    return true;
}

bool HealthMonitor::CheckMLOptimizerHealth() {
    // Check ML optimizer metrics
    if (storage_system_ && storage_system_->GetMLOptimizer()) {
        double efficiency = storage_system_->GetMLOptimizer()->GetStorageEfficiency();
        UpdateMetric("ml_optimizer_efficiency", efficiency);
    }
    return true;
}

bool HealthMonitor::CheckCloudIntegrationHealth() {
    // Check cloud integration status
    return true;
}

void HealthMonitor::SetThreshold(const std::string& metric_name, double warning, double critical) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    for (auto& metric : health_metrics_) {
        if (metric.name == metric_name) {
            metric.threshold_warning = warning;
            metric.threshold_critical = critical;
            break;
        }
    }
}

void HealthMonitor::ResetThresholds() {
    thresholds_ = Thresholds();
}

size_t HealthMonitor::GetTotalAlerts() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    return alert_history_.size();
}

size_t HealthMonitor::GetAlertsOfSeverity(AlertSeverity severity) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    return std::count_if(alert_history_.begin(), alert_history_.end(),
        [severity](const SystemAlert& alert) { return alert.severity == severity; });
}

double HealthMonitor::GetUptimeSeconds() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    return duration.count();
}

void HealthMonitor::MonitoringLoop() {
    while (running_) {
        CheckSystemHealth();
        CheckStorageHealth();
        CheckCompressionHealth();
        CheckMLOptimizerHealth();
        CheckCloudIntegrationHealth();
        
        std::this_thread::sleep_for(check_interval_);
    }
}

void HealthMonitor::CheckCPUUsage() {
    double cpu_usage = GetCPUUsage();
    UpdateMetric("cpu_usage", cpu_usage);
}

void HealthMonitor::CheckMemoryUsage() {
    double memory_usage = GetMemoryUsage();
    UpdateMetric("memory_usage", memory_usage);
}

void HealthMonitor::CheckDiskUsage() {
    double disk_usage = GetDiskUsage();
    UpdateMetric("disk_usage", disk_usage);
}

void HealthMonitor::CheckCompressionRatio() {
    if (storage_system_ && storage_system_->GetCompressionSystem()) {
        // Use storage efficiency as a proxy for compression ratio
        double efficiency = storage_system_->GetStorageEfficiency();
        UpdateMetric("compression_ratio", efficiency * 2.0); // Approximate ratio
    }
}

void HealthMonitor::CheckSpaceMultiplier() {
    if (storage_system_) {
        double multiplier = storage_system_->GetSpaceMultiplier();
        UpdateMetric("space_multiplier", multiplier);
    }
}

void HealthMonitor::CheckCacheHitRatio() {
    if (storage_system_ && storage_system_->GetVirtualManager()) {
        // Use a default value as the method is not yet implemented
        double ratio = 0.75; // Placeholder: assume 75% hit ratio
        UpdateMetric("cache_hit_ratio", ratio * 100.0);
    }
}

void HealthMonitor::CheckSystemTemperature() {
    // Placeholder for temperature monitoring
    // Would need platform-specific implementations
}

HealthStatus HealthMonitor::DetermineHealthStatus(double value, double warning_threshold, double critical_threshold, bool higher_is_worse) {
    if (higher_is_worse) {
        if (critical_threshold > 0.0 && value >= critical_threshold) {
            return HealthStatus::CRITICAL;
        } else if (value >= warning_threshold) {
            return HealthStatus::WARNING;
        }
    } else {
        // For metrics where higher is better
        if (value < warning_threshold) {
            return HealthStatus::WARNING;
        }
    }
    
    return HealthStatus::HEALTHY;
}

void HealthMonitor::NotifyAlert(const SystemAlert& alert) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    if (alert_callback_) {
        alert_callback_(alert);
    }
}

std::string HealthMonitor::HealthStatusToString(HealthStatus status) {
    switch (status) {
        case HealthStatus::HEALTHY: return "HEALTHY";
        case HealthStatus::WARNING: return "WARNING";
        case HealthStatus::CRITICAL: return "CRITICAL";
        case HealthStatus::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

std::string HealthMonitor::AlertSeverityToString(AlertSeverity severity) {
    switch (severity) {
        case AlertSeverity::INFO: return "INFO";
        case AlertSeverity::WARNING: return "WARNING";
        case AlertSeverity::ERROR: return "ERROR";
        case AlertSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

double HealthMonitor::GetCPUUsage() {
    // Simplified CPU usage estimation
    // In production, would use platform-specific APIs
    
#ifdef __linux__
    static unsigned long long prev_idle = 0, prev_total = 0;
    
    std::ifstream proc_stat("/proc/stat");
    if (proc_stat.is_open()) {
        std::string cpu;
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        
        proc_stat >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        proc_stat.close();
        
        unsigned long long idle_time = idle + iowait;
        unsigned long long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
        
        unsigned long long idle_delta = idle_time - prev_idle;
        unsigned long long total_delta = total_time - prev_total;
        
        prev_idle = idle_time;
        prev_total = total_time;
        
        if (total_delta > 0) {
            return 100.0 * (1.0 - (double)idle_delta / total_delta);
        }
    }
#endif
    
    return 0.0; // Placeholder
}

double HealthMonitor::GetMemoryUsage() {
    // Simplified memory usage estimation
    
#ifdef __linux__
    std::ifstream proc_meminfo("/proc/meminfo");
    if (proc_meminfo.is_open()) {
        std::string line;
        unsigned long long total = 0, available = 0;
        
        while (std::getline(proc_meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                sscanf(line.c_str(), "MemTotal: %llu", &total);
            } else if (line.find("MemAvailable:") == 0) {
                sscanf(line.c_str(), "MemAvailable: %llu", &available);
            }
        }
        proc_meminfo.close();
        
        if (total > 0) {
            return 100.0 * (1.0 - (double)available / total);
        }
    }
#endif
    
    return 0.0; // Placeholder
}

double HealthMonitor::GetDiskUsage() {
    // Simplified disk usage calculation
    if (storage_system_) {
        size_t used = storage_system_->GetPhysicalSpaceUsed();
        size_t total = storage_system_->GetVirtualSpaceTotal();
        
        if (total > 0) {
            return 100.0 * (double)used / total;
        }
    }
    
    return 0.0;
}

} // namespace StorageOpt
