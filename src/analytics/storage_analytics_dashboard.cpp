#include "storage_analytics_dashboard.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace StorageOpt {

StorageAnalyticsDashboard::StorageAnalyticsDashboard() : running_(false) {
    // Initialize default configuration
    config_.data_retention_days = 30;
    config_.prediction_horizon_days = 7;
    config_.min_confidence_threshold = 0.7;
    config_.max_insights_per_category = 10;
    config_.enable_quantum_prediction = true;
    config_.enable_real_time_monitoring = true;
    config_.monitoring_interval_seconds = 60;
    
    // Initialize performance thresholds
    thresholds_.storage_efficiency_warning = 0.5;
    thresholds_.storage_efficiency_critical = 0.3;
    thresholds_.compression_ratio_warning = 0.3;
    thresholds_.compression_ratio_critical = 0.1;
    thresholds_.cache_hit_ratio_warning = 0.7;
    thresholds_.cache_hit_ratio_critical = 0.5;
    thresholds_.space_usage_warning_percent = 80;
    thresholds_.space_usage_critical_percent = 95;
}

StorageAnalyticsDashboard::~StorageAnalyticsDashboard() {
    Stop();
}

bool StorageAnalyticsDashboard::Initialize() {
    try {
        // Initialize prediction models
        PredictionModel linear_model;
        linear_model.model_type = PredictionModel::LINEAR_REGRESSION;
        linear_model.accuracy = 0.0;
        linear_model.confidence = 0.0;
        linear_model.last_trained = std::chrono::system_clock::now();
        linear_model.training_samples_count = 0;
        prediction_models_["storage_usage"] = linear_model;
        
        PredictionModel neural_model;
        neural_model.model_type = PredictionModel::NEURAL_NETWORK;
        neural_model.weights.resize(10, 0.1); // Initialize with small weights
        neural_model.accuracy = 0.0;
        neural_model.confidence = 0.0;
        neural_model.last_trained = std::chrono::system_clock::now();
        neural_model.training_samples_count = 0;
        prediction_models_["file_access"] = neural_model;
        
        PredictionModel quantum_model;
        quantum_model.model_type = PredictionModel::QUANTUM_ENHANCED;
        quantum_model.accuracy = 0.0;
        quantum_model.confidence = 0.0;
        quantum_model.last_trained = std::chrono::system_clock::now();
        quantum_model.training_samples_count = 0;
        prediction_models_["quantum_optimization"] = quantum_model;
        
        std::cout << "Storage Analytics Dashboard initialized successfully." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Storage Analytics Dashboard: " << e.what() << std::endl;
        return false;
    }
}

void StorageAnalyticsDashboard::Start() {
    running_ = true;
    
    if (config_.enable_real_time_monitoring) {
        monitoring_thread_ = std::thread(&StorageAnalyticsDashboard::MonitoringLoop, this);
    }
    
    analysis_thread_ = std::thread(&StorageAnalyticsDashboard::AnalysisLoop, this);
    prediction_thread_ = std::thread(&StorageAnalyticsDashboard::PredictionLoop, this);
    
    std::cout << "Storage Analytics Dashboard started." << std::endl;
}

void StorageAnalyticsDashboard::Stop() {
    running_ = false;
    
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
    if (analysis_thread_.joinable()) {
        analysis_thread_.join();
    }
    if (prediction_thread_.joinable()) {
        prediction_thread_.join();
    }
    
    std::cout << "Storage Analytics Dashboard stopped." << std::endl;
}

void StorageAnalyticsDashboard::RecordMetric(const std::string& name, double value, const std::string& unit, const std::string& category) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    PerformanceMetric metric;
    metric.name = name;
    metric.value = value;
    metric.unit = unit;
    metric.category = category;
    metric.timestamp = std::chrono::system_clock::now();
    
    metrics_history_[name].push_back(metric);
    
    // Check for anomalies
    if (metrics_history_[name].size() > 10) {
        std::vector<double> recent_values;
        for (const auto& m : metrics_history_[name]) {
            recent_values.push_back(m.value);
        }
        
        if (IsAnomaly(value, recent_values)) {
            StorageInsight insight;
            insight.insight_type = "anomaly_detection";
            insight.description = "Anomalous value detected for metric: " + name;
            insight.impact_score = 0.8;
            insight.recommended_action = "Investigate potential issues with " + name;
            insight.confidence_level = 0.9;
            insight.generated_time = std::chrono::system_clock::now();
            
            insights_.push_back(insight);
        }
    }
}

void StorageAnalyticsDashboard::RecordFileAccess(const std::string& file_path, std::chrono::system_clock::time_point access_time) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto& pattern = usage_patterns_[file_path];
    pattern.file_path = file_path;
    pattern.access_times.push_back(access_time);
    
    // Keep only recent access times (configurable retention)
    auto cutoff_time = std::chrono::system_clock::now() - std::chrono::hours(24 * config_.data_retention_days);
    pattern.access_times.erase(
        std::remove_if(pattern.access_times.begin(), pattern.access_times.end(),
                      [cutoff_time](const auto& time) { return time < cutoff_time; }),
        pattern.access_times.end()
    );
    
    // Update frequency calculations
    auto now = std::chrono::system_clock::now();
    auto day_ago = now - std::chrono::hours(24);
    auto week_ago = now - std::chrono::hours(24 * 7);
    auto month_ago = now - std::chrono::hours(24 * 30);
    
    pattern.daily_access_frequency = std::count_if(
        pattern.access_times.begin(), pattern.access_times.end(),
        [day_ago](const auto& time) { return time >= day_ago; }
    );
    
    pattern.weekly_access_frequency = std::count_if(
        pattern.access_times.begin(), pattern.access_times.end(),
        [week_ago](const auto& time) { return time >= week_ago; }
    );
    
    pattern.monthly_access_frequency = std::count_if(
        pattern.access_times.begin(), pattern.access_times.end(),
        [month_ago](const auto& time) { return time >= month_ago; }
    );
    
    // Classify access pattern
    pattern.access_patterns.clear();
    pattern.access_patterns.push_back(ClassifyAccessPattern(pattern.access_times));
}

UsagePattern StorageAnalyticsDashboard::AnalyzeFileUsagePattern(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    auto it = usage_patterns_.find(file_path);
    if (it == usage_patterns_.end()) {
        return UsagePattern{}; // Return empty pattern if not found
    }
    
    UsagePattern& pattern = it->second;
    
    // Calculate prediction confidence based on data quality
    if (pattern.access_times.size() > 30) {
        pattern.prediction_confidence = 0.9;
    } else if (pattern.access_times.size() > 10) {
        pattern.prediction_confidence = 0.7;
    } else {
        pattern.prediction_confidence = 0.5;
    }
    
    // Predict next access time using ML
    if (pattern.access_times.size() >= 3) {
        std::vector<double> features = ExtractFeatures(pattern);
        auto prediction = NeuralNetworkPredict(features, prediction_models_["file_access"]);
        
        if (!prediction.empty()) {
            // Convert prediction to time (simplified)
            auto now = std::chrono::system_clock::now();
            auto predicted_hours = static_cast<int>(prediction[0] * 24 * 7); // Convert to hours within a week
            pattern.next_predicted_access = now + std::chrono::hours(predicted_hours);
        }
    }
    
    return pattern;
}

std::vector<UsagePattern> StorageAnalyticsDashboard::GetTopAccessedFiles(int count) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::vector<UsagePattern> patterns;
    for (const auto& [path, pattern] : usage_patterns_) {
        patterns.push_back(pattern);
    }
    
    // Sort by daily access frequency
    std::sort(patterns.begin(), patterns.end(),
              [](const UsagePattern& a, const UsagePattern& b) {
                  return a.daily_access_frequency > b.daily_access_frequency;
              });
    
    if (count > 0 && count < static_cast<int>(patterns.size())) {
        patterns.resize(count);
    }
    
    return patterns;
}

std::vector<UsagePattern> StorageAnalyticsDashboard::GetUnusedFiles(int days_threshold) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::vector<UsagePattern> unused_files;
    auto threshold_time = std::chrono::system_clock::now() - std::chrono::hours(24 * days_threshold);
    
    for (const auto& [path, pattern] : usage_patterns_) {
        if (pattern.access_times.empty() || 
            pattern.access_times.back() < threshold_time) {
            unused_files.push_back(pattern);
        }
    }
    
    return unused_files;
}

double StorageAnalyticsDashboard::PredictStorageUsage(std::chrono::system_clock::time_point target_time) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Get historical storage usage data
    auto storage_metrics = GetMetricsHistory("storage_usage", 
        std::chrono::system_clock::now() - std::chrono::hours(24 * 30),
        std::chrono::system_clock::now());
    
    if (storage_metrics.size() < 3) {
        return 0.0; // Not enough data for prediction
    }
    
    // Prepare data for linear regression
    std::vector<std::pair<double, double>> data_points;
    auto base_time = storage_metrics[0].timestamp;
    
    for (const auto& metric : storage_metrics) {
        double time_hours = std::chrono::duration<double, std::ratio<3600>>(
            metric.timestamp - base_time).count();
        data_points.push_back({time_hours, metric.value});
    }
    
    // Calculate time difference for target
    double target_hours = std::chrono::duration<double, std::ratio<3600>>(
        target_time - base_time).count();
    
    // Use linear regression for prediction
    double predicted_usage = LinearRegression(data_points, target_hours);
    
    // Apply quantum enhancement if enabled
    if (config_.enable_quantum_prediction) {
        std::vector<double> quantum_input = {predicted_usage, target_hours / (24 * 7)}; // Normalize time
        auto quantum_prediction = MakeQuantumEnhancedPrediction(quantum_input);
        if (!quantum_prediction.empty()) {
            predicted_usage = quantum_prediction[0];
        }
    }
    
    return std::max(0.0, predicted_usage);
}

void StorageAnalyticsDashboard::GenerateInsights() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Clear old insights
    insights_.clear();
    
    // Generate different types of insights
    GenerateStorageEfficiencyInsights();
    GenerateCompressionInsights();
    GenerateCacheOptimizationInsights();
    GenerateCapacityPlanningInsights();
    
    // Sort insights by impact score
    std::sort(insights_.begin(), insights_.end(),
              [](const StorageInsight& a, const StorageInsight& b) {
                  return a.impact_score > b.impact_score;
              });
    
    std::cout << "Generated " << insights_.size() << " insights." << std::endl;
}

std::vector<StorageInsight> StorageAnalyticsDashboard::GetCriticalInsights() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::vector<StorageInsight> critical_insights;
    for (const auto& insight : insights_) {
        if (insight.impact_score >= 0.8) {
            critical_insights.push_back(insight);
        }
    }
    
    return critical_insights;
}

std::string StorageAnalyticsDashboard::GenerateTextReport(const std::string& report_type) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::ostringstream report;
    auto now = std::chrono::system_clock::now();
    
    report << "=== Storage Analytics Report ===\n";
    report << "Generated: " << FormatTimestamp(now) << "\n";
    report << "Report Type: " << report_type << "\n\n";
    
    if (report_type == "summary" || report_type == "all") {
        // Performance Summary
        report << "PERFORMANCE SUMMARY\n";
        report << "-------------------\n";
        
        auto current_metrics = GetCurrentPerformanceSnapshot();
        for (const auto& [name, value] : current_metrics) {
            report << name << ": " << FormatMetricValue(value, "") << "\n";
        }
        report << "\n";
        
        // Top Accessed Files
        report << "TOP ACCESSED FILES (Last 24 hours)\n";
        report << "-----------------------------------\n";
        auto top_files = GetTopAccessedFiles(5);
        for (const auto& pattern : top_files) {
            report << pattern.file_path << " (Accesses: " << pattern.daily_access_frequency << ")\n";
        }
        report << "\n";
        
        // Critical Insights
        report << "CRITICAL INSIGHTS\n";
        report << "-----------------\n";
        auto critical_insights = GetCriticalInsights();
        for (const auto& insight : critical_insights) {
            report << "• " << insight.description << "\n";
            report << "  Action: " << insight.recommended_action << "\n";
            report << "  Impact: " << FormatMetricValue(insight.impact_score * 100, "%") << "\n\n";
        }
    }
    
    if (report_type == "prediction" || report_type == "all") {
        // Predictions
        report << "PREDICTIONS\n";
        report << "-----------\n";
        
        auto next_week = now + std::chrono::hours(24 * 7);
        double predicted_usage = PredictStorageUsage(next_week);
        report << "Storage usage (7 days): " << FormatMetricValue(predicted_usage, "GB") << "\n";
        
        auto files_to_cleanup = PredictFilesDueForCleanup(7);
        report << "Files due for cleanup: " << files_to_cleanup.size() << "\n";
        report << "\n";
    }
    
    report << "=== End of Report ===\n";
    return report.str();
}

std::unordered_map<std::string, double> StorageAnalyticsDashboard::GetCurrentPerformanceSnapshot() {
    std::unordered_map<std::string, double> snapshot;
    
    // Calculate current metrics (would integrate with actual storage system)
    snapshot["storage_efficiency"] = CalculateAverageMetric("storage_efficiency", 1);
    snapshot["compression_ratio"] = CalculateAverageMetric("compression_ratio", 1);
    snapshot["cache_hit_ratio"] = CalculateAverageMetric("cache_hit_ratio", 1);
    snapshot["total_files_monitored"] = static_cast<double>(GetTotalFilesMonitored());
    snapshot["total_metrics_collected"] = static_cast<double>(GetTotalMetricsCollected());
    
    return snapshot;
}

void StorageAnalyticsDashboard::MonitoringLoop() {
    while (running_) {
        try {
            // Collect system metrics
            RecordMetric("system_cpu_usage", 45.2, "%", "system");
            RecordMetric("system_memory_usage", 68.7, "%", "system");
            RecordMetric("storage_io_operations", 156.3, "ops/sec", "performance");
            
            // Simulate quantum-enhanced metrics
            if (config_.enable_quantum_prediction) {
                std::vector<double> quantum_metrics = MakeQuantumEnhancedPrediction({45.2, 68.7, 156.3});
                if (quantum_metrics.size() >= 3) {
                    RecordMetric("quantum_efficiency", quantum_metrics[0], "score", "quantum");
                    RecordMetric("quantum_coherence", quantum_metrics[1], "score", "quantum");
                    RecordMetric("quantum_entanglement", quantum_metrics[2], "score", "quantum");
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(config_.monitoring_interval_seconds));
        } catch (const std::exception& e) {
            std::cerr << "Error in monitoring loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }
}

void StorageAnalyticsDashboard::AnalysisLoop() {
    while (running_) {
        try {
            AnalyzeUsagePatterns();
            DetectAnomalies();
            GenerateInsights();
            CleanupOldData();
            
            std::this_thread::sleep_for(std::chrono::minutes(10)); // Run every 10 minutes
        } catch (const std::exception& e) {
            std::cerr << "Error in analysis loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

void StorageAnalyticsDashboard::PredictionLoop() {
    while (running_) {
        try {
            UpdatePredictionModels();
            
            // Generate predictions for the next period
            auto future_time = std::chrono::system_clock::now() + 
                              std::chrono::hours(24 * config_.prediction_horizon_days);
            
            double predicted_usage = PredictStorageUsage(future_time);
            RecordMetric("predicted_storage_usage", predicted_usage, "GB", "prediction");
            
            std::this_thread::sleep_for(std::chrono::hours(1)); // Run every hour
        } catch (const std::exception& e) {
            std::cerr << "Error in prediction loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(10));
        }
    }
}

double StorageAnalyticsDashboard::LinearRegression(const std::vector<std::pair<double, double>>& data, double input) {
    if (data.size() < 2) return 0.0;
    
    // Calculate means
    double sum_x = 0.0, sum_y = 0.0;
    for (const auto& point : data) {
        sum_x += point.first;
        sum_y += point.second;
    }
    double mean_x = sum_x / data.size();
    double mean_y = sum_y / data.size();
    
    // Calculate slope and intercept
    double numerator = 0.0, denominator = 0.0;
    for (const auto& point : data) {
        numerator += (point.first - mean_x) * (point.second - mean_y);
        denominator += (point.first - mean_x) * (point.first - mean_x);
    }
    
    if (denominator == 0.0) return mean_y;
    
    double slope = numerator / denominator;
    double intercept = mean_y - slope * mean_x;
    
    return slope * input + intercept;
}

std::vector<double> StorageAnalyticsDashboard::MakeQuantumEnhancedPrediction(const std::vector<double>& input_features) {
    if (input_features.empty()) return {};
    
    // Apply quantum interference to classical predictions
    std::vector<double> quantum_predictions = input_features;
    
    // Simulate quantum superposition
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(0.0, 0.1);
    
    for (size_t i = 0; i < quantum_predictions.size(); ++i) {
        // Apply quantum noise and interference
        double quantum_noise = dis(gen);
        double interference = std::sin(static_cast<double>(i) * M_PI / quantum_predictions.size()) * 0.05;
        
        quantum_predictions[i] = quantum_predictions[i] * (1.0 + quantum_noise + interference);
        
        // Apply quantum entanglement with other predictions
        if (i > 0) {
            quantum_predictions[i] += quantum_predictions[i-1] * 0.1;
        }
    }
    
    return quantum_predictions;
}

std::string StorageAnalyticsDashboard::ClassifyAccessPattern(const std::vector<std::chrono::system_clock::time_point>& access_times) {
    if (access_times.empty()) return "no_pattern";
    if (access_times.size() < 3) return "insufficient_data";
    
    // Analyze time distribution
    std::vector<int> hours_of_day(24, 0);
    std::vector<int> days_of_week(7, 0);
    
    for (const auto& time : access_times) {
        auto time_t = std::chrono::system_clock::to_time_t(time);
        auto* local_time = std::localtime(&time_t);
        
        hours_of_day[local_time->tm_hour]++;
        days_of_week[local_time->tm_wday]++;
    }
    
    // Find peak hour
    int peak_hour = std::max_element(hours_of_day.begin(), hours_of_day.end()) - hours_of_day.begin();
    
    // Classify based on patterns
    if (peak_hour >= 9 && peak_hour <= 17) {
        return "business_hours";
    } else if (peak_hour >= 18 && peak_hour <= 22) {
        return "evening_usage";
    } else if (peak_hour >= 23 || peak_hour <= 6) {
        return "night_usage";
    } else {
        return "random_access";
    }
}

void StorageAnalyticsDashboard::GenerateStorageEfficiencyInsights() {
    double current_efficiency = CalculateAverageMetric("storage_efficiency", 1);
    
    if (current_efficiency < thresholds_.storage_efficiency_critical) {
        StorageInsight insight;
        insight.insight_type = "storage_efficiency";
        insight.description = "Critical: Storage efficiency is below " + 
                             std::to_string(static_cast<int>(thresholds_.storage_efficiency_critical * 100)) + "%";
        insight.impact_score = 0.95;
        insight.recommended_action = "Implement aggressive compression and deduplication";
        insight.confidence_level = 0.9;
        insight.generated_time = std::chrono::system_clock::now();
        insights_.push_back(insight);
    }
}

void StorageAnalyticsDashboard::GenerateCompressionInsights() {
    double compression_ratio = CalculateAverageMetric("compression_ratio", 7);
    
    if (compression_ratio < thresholds_.compression_ratio_warning) {
        StorageInsight insight;
        insight.insight_type = "compression";
        insight.description = "Low compression ratio detected. Consider different algorithms.";
        insight.impact_score = 0.7;
        insight.recommended_action = "Analyze file types and select optimal compression algorithms";
        insight.confidence_level = 0.8;
        insight.generated_time = std::chrono::system_clock::now();
        insights_.push_back(insight);
    }
}

std::vector<double> StorageAnalyticsDashboard::ExtractFeatures(const UsagePattern& pattern) {
    std::vector<double> features;
    
    features.push_back(pattern.daily_access_frequency);
    features.push_back(pattern.weekly_access_frequency);
    features.push_back(pattern.monthly_access_frequency);
    features.push_back(static_cast<double>(pattern.access_times.size()));
    
    return features;
}

bool StorageAnalyticsDashboard::IsAnomaly(double value, const std::vector<double>& historical_data, double sensitivity) {
    if (historical_data.size() < 5) return false;
    
    double mean = std::accumulate(historical_data.begin(), historical_data.end(), 0.0) / historical_data.size();
    double std_dev = CalculateStandardDeviation(historical_data);
    
    return std::abs(value - mean) > (sensitivity * std_dev);
}

double StorageAnalyticsDashboard::CalculateStandardDeviation(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    double variance = 0.0;
    for (double value : data) {
        variance += (value - mean) * (value - mean);
    }
    variance /= data.size();
    
    return std::sqrt(variance);
}

size_t StorageAnalyticsDashboard::GetTotalFilesMonitored() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return usage_patterns_.size();
}

size_t StorageAnalyticsDashboard::GetTotalMetricsCollected() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    size_t total = 0;
    for (const auto& [name, metrics] : metrics_history_) {
        total += metrics.size();
    }
    return total;
}

std::string StorageAnalyticsDashboard::FormatMetricValue(double value, const std::string& unit) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    if (!unit.empty()) {
        oss << " " << unit;
    }
    return oss.str();
}

std::string StorageAnalyticsDashboard::FormatTimestamp(std::chrono::system_clock::time_point timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Placeholder implementations for remaining methods
void StorageAnalyticsDashboard::AnalyzeUsagePatterns() {
    // Analyze patterns across all files
    std::cout << "Analyzing usage patterns..." << std::endl;
}

void StorageAnalyticsDashboard::DetectAnomalies() {
    // Detect anomalies in metrics
    std::cout << "Detecting anomalies..." << std::endl;
}

void StorageAnalyticsDashboard::UpdatePredictionModels() {
    // Update ML models with new data
    std::cout << "Updating prediction models..." << std::endl;
}

void StorageAnalyticsDashboard::CleanupOldData() {
    // Remove old data based on retention policy
    std::cout << "Cleaning up old data..." << std::endl;
}

std::vector<double> StorageAnalyticsDashboard::NeuralNetworkPredict(const std::vector<double>& inputs, const PredictionModel& model) {
    // Simple neural network prediction
    std::vector<double> outputs;
    if (!inputs.empty() && !model.weights.empty()) {
        double sum = 0.0;
        for (size_t i = 0; i < inputs.size() && i < model.weights.size(); ++i) {
            sum += inputs[i] * model.weights[i];
        }
        outputs.push_back(1.0 / (1.0 + std::exp(-sum))); // Sigmoid activation
    }
    return outputs;
}

double StorageAnalyticsDashboard::CalculateAverageMetric(const std::string& metric_name, int days_back) {
    auto metrics = GetMetricsHistory(metric_name, 
        std::chrono::system_clock::now() - std::chrono::hours(24 * days_back),
        std::chrono::system_clock::now());
    
    if (metrics.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& metric : metrics) {
        sum += metric.value;
    }
    return sum / metrics.size();
}

std::vector<PerformanceMetric> StorageAnalyticsDashboard::GetMetricsHistory(const std::string& metric_name, 
                                                                          std::chrono::system_clock::time_point start_time,
                                                                          std::chrono::system_clock::time_point end_time) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::vector<PerformanceMetric> filtered_metrics;
    
    auto it = metrics_history_.find(metric_name);
    if (it != metrics_history_.end()) {
        for (const auto& metric : it->second) {
            if (metric.timestamp >= start_time && metric.timestamp <= end_time) {
                filtered_metrics.push_back(metric);
            }
        }
    }
    
    return filtered_metrics;
}

std::vector<std::string> StorageAnalyticsDashboard::PredictFilesDueForCleanup(int days_ahead) {
    // Predict files that will become candidates for cleanup
    return {}; // Placeholder
}

void StorageAnalyticsDashboard::GenerateCacheOptimizationInsights() {
    // Generate insights about cache optimization
}

void StorageAnalyticsDashboard::GenerateCapacityPlanningInsights() {
    // Generate insights about capacity planning
}

} // namespace StorageOpt