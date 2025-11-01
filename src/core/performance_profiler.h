#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <mutex>
#include <functional>

namespace StorageOpt {

// Forward declaration
class QuantumStorageSystem;

struct ProfilerMetric {
    std::string operation_name;
    size_t call_count;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    size_t total_bytes_processed;
    double throughput_mbps;
};

struct BenchmarkResult {
    std::string benchmark_name;
    double execution_time_ms;
    size_t operations_count;
    double operations_per_second;
    size_t bytes_processed;
    double throughput_mbps;
    bool success;
    std::string error_message;
    std::map<std::string, double> custom_metrics;
};

class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    bool running_;

public:
    PerformanceTimer() : running_(false) {}
    
    void Start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
    }
    
    double Stop() {
        if (!running_) return 0.0;
        auto end_time = std::chrono::high_resolution_clock::now();
        running_ = false;
        return std::chrono::duration<double, std::milli>(end_time - start_time_).count();
    }
    
    double Elapsed() const {
        if (!running_) return 0.0;
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(current_time - start_time_).count();
    }
};

class PerformanceProfiler {
private:
    QuantumStorageSystem* storage_system_;
    
    // Performance tracking
    std::map<std::string, ProfilerMetric> metrics_;
    std::mutex metrics_mutex_;
    
    // Benchmark results
    std::vector<BenchmarkResult> benchmark_history_;
    std::mutex benchmark_mutex_;
    
    bool profiling_enabled_;
    std::chrono::system_clock::time_point profiling_start_time_;

public:
    PerformanceProfiler(QuantumStorageSystem* system);
    ~PerformanceProfiler();
    
    // Profiling control
    void EnableProfiling();
    void DisableProfiling();
    bool IsProfilingEnabled() const { return profiling_enabled_; }
    void ResetMetrics();
    
    // Performance tracking
    void RecordOperation(const std::string& operation_name, double time_ms, size_t bytes_processed = 0);
    ProfilerMetric GetMetric(const std::string& operation_name);
    std::vector<ProfilerMetric> GetAllMetrics();
    
    // Automatic profiling helpers
    class ScopedProfile {
    private:
        PerformanceProfiler* profiler_;
        std::string operation_name_;
        PerformanceTimer timer_;
        size_t bytes_;
        
    public:
        ScopedProfile(PerformanceProfiler* profiler, const std::string& operation_name, size_t bytes = 0)
            : profiler_(profiler), operation_name_(operation_name), bytes_(bytes) {
            if (profiler_ && profiler_->IsProfilingEnabled()) {
                timer_.Start();
            }
        }
        
        ~ScopedProfile() {
            if (profiler_ && profiler_->IsProfilingEnabled()) {
                double elapsed = timer_.Stop();
                profiler_->RecordOperation(operation_name_, elapsed, bytes_);
            }
        }
    };
    
    // Benchmarks
    BenchmarkResult BenchmarkFileCreation(size_t file_count, size_t file_size);
    BenchmarkResult BenchmarkFileWrite(size_t write_count, size_t write_size);
    BenchmarkResult BenchmarkFileRead(size_t read_count, size_t read_size);
    BenchmarkResult BenchmarkCompression(const std::vector<uint8_t>& data);
    BenchmarkResult BenchmarkEncryption(const std::vector<uint8_t>& data);
    BenchmarkResult BenchmarkBatchOperations(size_t operation_count);
    BenchmarkResult BenchmarkSystemThroughput(size_t duration_seconds);
    
    // Custom benchmarks
    using BenchmarkFunction = std::function<void()>;
    BenchmarkResult RunCustomBenchmark(const std::string& name, BenchmarkFunction func, size_t iterations = 1);
    
    // Analysis
    std::string GeneratePerformanceReport();
    std::string GenerateBenchmarkReport();
    std::vector<BenchmarkResult> GetBenchmarkHistory();
    
    // Export
    bool ExportMetricsToCSV(const std::string& filename);
    bool ExportBenchmarksToCSV(const std::string& filename);
    
private:
    void UpdateMetric(const std::string& operation_name, double time_ms, size_t bytes_processed);
    double CalculateThroughput(size_t bytes, double time_ms);
};

} // namespace StorageOpt
