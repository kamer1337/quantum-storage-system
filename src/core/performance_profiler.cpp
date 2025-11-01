#include "performance_profiler.h"
#include "../quantum_storage_system.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <numeric>

namespace StorageOpt {

PerformanceProfiler::PerformanceProfiler(QuantumStorageSystem* system)
    : storage_system_(system)
    , profiling_enabled_(false)
    , profiling_start_time_(std::chrono::system_clock::now()) {
}

PerformanceProfiler::~PerformanceProfiler() {
}

void PerformanceProfiler::EnableProfiling() {
    profiling_enabled_ = true;
    profiling_start_time_ = std::chrono::system_clock::now();
}

void PerformanceProfiler::DisableProfiling() {
    profiling_enabled_ = false;
}

void PerformanceProfiler::ResetMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.clear();
    profiling_start_time_ = std::chrono::system_clock::now();
}

void PerformanceProfiler::RecordOperation(const std::string& operation_name, double time_ms, size_t bytes_processed) {
    if (!profiling_enabled_) return;
    
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    UpdateMetric(operation_name, time_ms, bytes_processed);
}

ProfilerMetric PerformanceProfiler::GetMetric(const std::string& operation_name) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto it = metrics_.find(operation_name);
    if (it != metrics_.end()) {
        return it->second;
    }
    
    return {"", 0, 0.0, 0.0, 0.0, 0.0, 0, 0.0};
}

std::vector<ProfilerMetric> PerformanceProfiler::GetAllMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<ProfilerMetric> all_metrics;
    for (const auto& pair : metrics_) {
        all_metrics.push_back(pair.second);
    }
    
    return all_metrics;
}

BenchmarkResult PerformanceProfiler::BenchmarkFileCreation(size_t file_count, size_t file_size) {
    BenchmarkResult result;
    result.benchmark_name = "File Creation";
    result.operations_count = file_count;
    result.bytes_processed = file_count * file_size;
    
    PerformanceTimer timer;
    timer.Start();
    
    size_t successful = 0;
    for (size_t i = 0; i < file_count; ++i) {
        std::string filename = "benchmark_file_" + std::to_string(i) + ".dat";
        if (storage_system_->CreateFile(filename, file_size)) {
            successful++;
        }
    }
    
    result.execution_time_ms = timer.Stop();
    result.success = (successful == file_count);
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (file_count * 1000.0 / result.execution_time_ms) : 0.0;
    result.throughput_mbps = CalculateThroughput(result.bytes_processed, result.execution_time_ms);
    
    if (!result.success) {
        result.error_message = "Failed to create " + std::to_string(file_count - successful) + " files";
    }
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkFileWrite(size_t write_count, size_t write_size) {
    BenchmarkResult result;
    result.benchmark_name = "File Write";
    result.operations_count = write_count;
    result.bytes_processed = write_count * write_size;
    
    // Create test file first
    std::string filename = "benchmark_write_test.dat";
    storage_system_->CreateFile(filename, write_count * write_size);
    
    std::vector<uint8_t> data(write_size, 0x42);
    
    PerformanceTimer timer;
    timer.Start();
    
    size_t successful = 0;
    for (size_t i = 0; i < write_count; ++i) {
        if (storage_system_->WriteFile(filename, data.data(), data.size())) {
            successful++;
        }
    }
    
    result.execution_time_ms = timer.Stop();
    result.success = (successful == write_count);
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (write_count * 1000.0 / result.execution_time_ms) : 0.0;
    result.throughput_mbps = CalculateThroughput(result.bytes_processed, result.execution_time_ms);
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkFileRead(size_t read_count, size_t read_size) {
    BenchmarkResult result;
    result.benchmark_name = "File Read";
    result.operations_count = read_count;
    result.bytes_processed = read_count * read_size;
    
    // Create and write test file first
    std::string filename = "benchmark_read_test.dat";
    storage_system_->CreateFile(filename, read_size);
    std::vector<uint8_t> write_data(read_size, 0x42);
    storage_system_->WriteFile(filename, write_data.data(), write_data.size());
    
    std::vector<uint8_t> read_buffer(read_size);
    
    PerformanceTimer timer;
    timer.Start();
    
    size_t successful = 0;
    for (size_t i = 0; i < read_count; ++i) {
        size_t size = read_buffer.size();
        if (storage_system_->ReadFile(filename, read_buffer.data(), size)) {
            successful++;
        }
    }
    
    result.execution_time_ms = timer.Stop();
    result.success = (successful == read_count);
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (read_count * 1000.0 / result.execution_time_ms) : 0.0;
    result.throughput_mbps = CalculateThroughput(result.bytes_processed, result.execution_time_ms);
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkCompression(const std::vector<uint8_t>& data) {
    BenchmarkResult result;
    result.benchmark_name = "Compression";
    result.operations_count = 1;
    result.bytes_processed = data.size();
    
    if (!storage_system_->GetCompressionSystem()) {
        result.success = false;
        result.error_message = "Compression system not available";
        return result;
    }
    
    PerformanceTimer timer;
    timer.Start();
    
    auto compress_result = storage_system_->GetCompressionSystem()->CompressData(data);
    
    result.execution_time_ms = timer.Stop();
    result.success = compress_result.success;
    result.throughput_mbps = CalculateThroughput(data.size(), result.execution_time_ms);
    
    if (result.success) {
        result.custom_metrics["compression_ratio"] = compress_result.compression_ratio;
        result.custom_metrics["compressed_size"] = static_cast<double>(compress_result.compressed_size);
    }
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkEncryption(const std::vector<uint8_t>& data) {
    BenchmarkResult result;
    result.benchmark_name = "Encryption";
    result.operations_count = 1;
    result.bytes_processed = data.size();
    
    if (!storage_system_->GetEncryptionManager()) {
        result.success = false;
        result.error_message = "Encryption manager not available";
        return result;
    }
    
    PerformanceTimer timer;
    timer.Start();
    
    auto encrypt_result = storage_system_->GetEncryptionManager()->EncryptData(data);
    
    result.execution_time_ms = timer.Stop();
    result.success = encrypt_result.success;
    result.throughput_mbps = CalculateThroughput(data.size(), result.execution_time_ms);
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkBatchOperations(size_t operation_count) {
    BenchmarkResult result;
    result.benchmark_name = "Batch Operations";
    result.operations_count = operation_count;
    
    if (!storage_system_->GetBatchOperationManager()) {
        result.success = false;
        result.error_message = "Batch operation manager not available";
        return result;
    }
    
    // Create batch file operations
    std::vector<std::pair<std::string, size_t>> files;
    for (size_t i = 0; i < operation_count; ++i) {
        files.push_back({"batch_test_" + std::to_string(i) + ".dat", 1024});
    }
    
    PerformanceTimer timer;
    timer.Start();
    
    auto batch_result = storage_system_->GetBatchOperationManager()->CreateFiles(files);
    
    result.execution_time_ms = timer.Stop();
    result.success = (batch_result.successful_operations == static_cast<int>(operation_count));
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (operation_count * 1000.0 / result.execution_time_ms) : 0.0;
    
    result.custom_metrics["successful_operations"] = static_cast<double>(batch_result.successful_operations);
    result.custom_metrics["failed_operations"] = static_cast<double>(batch_result.failed_operations);
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::BenchmarkSystemThroughput(size_t duration_seconds) {
    BenchmarkResult result;
    result.benchmark_name = "System Throughput";
    result.operations_count = 0;
    result.bytes_processed = 0;
    
    PerformanceTimer timer;
    timer.Start();
    
    size_t operations = 0;
    size_t bytes = 0;
    std::vector<uint8_t> data(1024, 0x42); // 1KB chunks
    
    while (timer.Elapsed() < duration_seconds * 1000.0) {
        std::string filename = "throughput_test_" + std::to_string(operations) + ".dat";
        if (storage_system_->CreateFile(filename, data.size())) {
            if (storage_system_->WriteFile(filename, data.data(), data.size())) {
                operations++;
                bytes += data.size();
            }
        }
    }
    
    result.execution_time_ms = timer.Stop();
    result.operations_count = operations;
    result.bytes_processed = bytes;
    result.success = true;
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (operations * 1000.0 / result.execution_time_ms) : 0.0;
    result.throughput_mbps = CalculateThroughput(bytes, result.execution_time_ms);
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

BenchmarkResult PerformanceProfiler::RunCustomBenchmark(const std::string& name, BenchmarkFunction func, size_t iterations) {
    BenchmarkResult result;
    result.benchmark_name = name;
    result.operations_count = iterations;
    
    PerformanceTimer timer;
    timer.Start();
    
    try {
        for (size_t i = 0; i < iterations; ++i) {
            func();
        }
        result.success = true;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }
    
    result.execution_time_ms = timer.Stop();
    result.operations_per_second = (result.execution_time_ms > 0) ? 
        (iterations * 1000.0 / result.execution_time_ms) : 0.0;
    
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    benchmark_history_.push_back(result);
    
    return result;
}

std::string PerformanceProfiler::GeneratePerformanceReport() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::stringstream ss;
    ss << "\n=== PERFORMANCE REPORT ===\n\n";
    
    if (metrics_.empty()) {
        ss << "No performance metrics recorded.\n";
        return ss.str();
    }
    
    ss << std::left << std::setw(30) << "Operation"
       << std::right << std::setw(10) << "Calls"
       << std::setw(15) << "Total (ms)"
       << std::setw(15) << "Avg (ms)"
       << std::setw(15) << "Min (ms)"
       << std::setw(15) << "Max (ms)"
       << std::setw(15) << "Throughput\n";
    ss << std::string(115, '-') << "\n";
    
    for (const auto& pair : metrics_) {
        const auto& metric = pair.second;
        ss << std::left << std::setw(30) << metric.operation_name
           << std::right << std::setw(10) << metric.call_count
           << std::setw(15) << std::fixed << std::setprecision(2) << metric.total_time_ms
           << std::setw(15) << metric.avg_time_ms
           << std::setw(15) << metric.min_time_ms
           << std::setw(15) << metric.max_time_ms
           << std::setw(12) << metric.throughput_mbps << " MB/s\n";
    }
    
    return ss.str();
}

std::string PerformanceProfiler::GenerateBenchmarkReport() {
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    
    std::stringstream ss;
    ss << "\n=== BENCHMARK REPORT ===\n\n";
    
    if (benchmark_history_.empty()) {
        ss << "No benchmarks recorded.\n";
        return ss.str();
    }
    
    for (const auto& result : benchmark_history_) {
        ss << "Benchmark: " << result.benchmark_name << "\n";
        ss << "  Status: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
        if (!result.success && !result.error_message.empty()) {
            ss << "  Error: " << result.error_message << "\n";
        }
        ss << "  Operations: " << result.operations_count << "\n";
        ss << "  Execution Time: " << std::fixed << std::setprecision(2) << result.execution_time_ms << " ms\n";
        ss << "  Operations/sec: " << std::fixed << std::setprecision(2) << result.operations_per_second << "\n";
        if (result.bytes_processed > 0) {
            ss << "  Bytes Processed: " << result.bytes_processed << "\n";
            ss << "  Throughput: " << std::fixed << std::setprecision(2) << result.throughput_mbps << " MB/s\n";
        }
        
        if (!result.custom_metrics.empty()) {
            ss << "  Custom Metrics:\n";
            for (const auto& metric : result.custom_metrics) {
                ss << "    " << metric.first << ": " << metric.second << "\n";
            }
        }
        ss << "\n";
    }
    
    return ss.str();
}

std::vector<BenchmarkResult> PerformanceProfiler::GetBenchmarkHistory() {
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    return benchmark_history_;
}

bool PerformanceProfiler::ExportMetricsToCSV(const std::string& filename) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "Operation,CallCount,TotalTimeMs,AvgTimeMs,MinTimeMs,MaxTimeMs,TotalBytes,ThroughputMBps\n";
    
    for (const auto& pair : metrics_) {
        const auto& m = pair.second;
        file << m.operation_name << ","
             << m.call_count << ","
             << m.total_time_ms << ","
             << m.avg_time_ms << ","
             << m.min_time_ms << ","
             << m.max_time_ms << ","
             << m.total_bytes_processed << ","
             << m.throughput_mbps << "\n";
    }
    
    file.close();
    return true;
}

bool PerformanceProfiler::ExportBenchmarksToCSV(const std::string& filename) {
    std::lock_guard<std::mutex> lock(benchmark_mutex_);
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "Benchmark,Success,OperationCount,ExecutionTimeMs,OperationsPerSec,BytesProcessed,ThroughputMBps\n";
    
    for (const auto& r : benchmark_history_) {
        file << r.benchmark_name << ","
             << (r.success ? "true" : "false") << ","
             << r.operations_count << ","
             << r.execution_time_ms << ","
             << r.operations_per_second << ","
             << r.bytes_processed << ","
             << r.throughput_mbps << "\n";
    }
    
    file.close();
    return true;
}

void PerformanceProfiler::UpdateMetric(const std::string& operation_name, double time_ms, size_t bytes_processed) {
    auto& metric = metrics_[operation_name];
    
    if (metric.operation_name.empty()) {
        metric.operation_name = operation_name;
        metric.call_count = 0;
        metric.total_time_ms = 0.0;
        metric.min_time_ms = time_ms;
        metric.max_time_ms = time_ms;
        metric.total_bytes_processed = 0;
    }
    
    metric.call_count++;
    metric.total_time_ms += time_ms;
    metric.min_time_ms = std::min(metric.min_time_ms, time_ms);
    metric.max_time_ms = std::max(metric.max_time_ms, time_ms);
    metric.avg_time_ms = metric.total_time_ms / metric.call_count;
    metric.total_bytes_processed += bytes_processed;
    
    if (metric.total_time_ms > 0) {
        metric.throughput_mbps = CalculateThroughput(metric.total_bytes_processed, metric.total_time_ms);
    }
}

double PerformanceProfiler::CalculateThroughput(size_t bytes, double time_ms) {
    if (time_ms <= 0.0) return 0.0;
    double megabytes = bytes / (1024.0 * 1024.0);
    double seconds = time_ms / 1000.0;
    return megabytes / seconds;
}

} // namespace StorageOpt
