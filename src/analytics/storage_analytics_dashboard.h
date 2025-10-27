#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>
#include <functional>
#include <atomic>

namespace StorageOpt {

struct PerformanceMetric {
    std::string name;
    double value;
    std::string unit;
    std::chrono::system_clock::time_point timestamp;
    std::string category;
};

struct UsagePattern {
    std::string file_path;
    std::vector<std::chrono::system_clock::time_point> access_times;
    double daily_access_frequency;
    double weekly_access_frequency;
    double monthly_access_frequency;
    std::vector<std::string> access_patterns; // e.g., "weekday_morning", "weekend_evening"
    double prediction_confidence;
    std::chrono::system_clock::time_point next_predicted_access;
};

struct StorageInsight {
    std::string insight_type;
    std::string description;
    double impact_score;
    std::vector<std::string> affected_files;
    std::string recommended_action;
    double confidence_level;
    std::chrono::system_clock::time_point generated_time;
};

struct PredictionModel {
    enum Type {
        LINEAR_REGRESSION,
        NEURAL_NETWORK,
        RANDOM_FOREST,
        QUANTUM_ENHANCED,
        ENSEMBLE
    };
    
    Type model_type;
    std::vector<double> weights;
    double accuracy;
    double confidence;
    std::chrono::system_clock::time_point last_trained;
    int training_samples_count;
};

class StorageAnalyticsDashboard {
private:
    // Data collection
    std::unordered_map<std::string, std::vector<PerformanceMetric>> metrics_history_;
    std::unordered_map<std::string, UsagePattern> usage_patterns_;
    std::vector<StorageInsight> insights_;
    std::mutex data_mutex_;
    
    // Real-time monitoring
    std::thread monitoring_thread_;
    std::thread analysis_thread_;
    std::thread prediction_thread_;
    std::atomic<bool> running_;
    
    // Machine learning models
    std::unordered_map<std::string, PredictionModel> prediction_models_;
    
    // Analytics configuration
    struct AnalyticsConfig {
        int data_retention_days = 30;
        int prediction_horizon_days = 7;
        double min_confidence_threshold = 0.7;
        int max_insights_per_category = 10;
        bool enable_quantum_prediction = true;
        bool enable_real_time_monitoring = true;
        int monitoring_interval_seconds = 60;
    } config_;
    
    // Performance thresholds
    struct PerformanceThresholds {
        double storage_efficiency_warning = 0.5;
        double storage_efficiency_critical = 0.3;
        double compression_ratio_warning = 0.3;
        double compression_ratio_critical = 0.1;
        double cache_hit_ratio_warning = 0.7;
        double cache_hit_ratio_critical = 0.5;
        size_t space_usage_warning_percent = 80;
        size_t space_usage_critical_percent = 95;
    } thresholds_;

public:
    StorageAnalyticsDashboard();
    ~StorageAnalyticsDashboard();
    
    // Initialization and lifecycle
    bool Initialize();
    void Start();
    void Stop();
    
    // Data collection
    void RecordMetric(const std::string& name, double value, const std::string& unit, const std::string& category = "general");
    void RecordFileAccess(const std::string& file_path, std::chrono::system_clock::time_point access_time);
    void RecordPerformanceEvent(const std::string& event_type, const std::unordered_map<std::string, double>& metrics);
    
    // Usage pattern analysis
    UsagePattern AnalyzeFileUsagePattern(const std::string& file_path);
    std::vector<UsagePattern> GetTopAccessedFiles(int count = 10);
    std::vector<UsagePattern> GetUnusedFiles(int days_threshold = 30);
    std::vector<std::string> PredictFilesToAccess(std::chrono::system_clock::time_point target_time);
    
    // Performance analytics
    std::vector<PerformanceMetric> GetMetricsHistory(const std::string& metric_name, 
                                                    std::chrono::system_clock::time_point start_time,
                                                    std::chrono::system_clock::time_point end_time);
    double CalculateAverageMetric(const std::string& metric_name, int days_back = 7);
    double CalculateTrendSlope(const std::string& metric_name, int days_back = 7);
    std::unordered_map<std::string, double> GetCurrentPerformanceSnapshot();
    
    // Predictive analytics
    double PredictStorageUsage(std::chrono::system_clock::time_point target_time);
    double PredictCompressionEfficiency(std::chrono::system_clock::time_point target_time);
    std::vector<std::string> PredictFilesDueForCleanup(int days_ahead = 7);
    double PredictOptimalCacheSize(std::chrono::system_clock::time_point target_time);
    
    // Machine learning
    void TrainPredictionModel(const std::string& model_name, PredictionModel::Type type);
    double GetModelAccuracy(const std::string& model_name);
    void UpdateModelWithFeedback(const std::string& model_name, const std::vector<double>& actual_values);
    std::vector<double> MakeQuantumEnhancedPrediction(const std::vector<double>& input_features);
    
    // Insights generation
    void GenerateInsights();
    std::vector<StorageInsight> GetInsights(const std::string& category = "");
    std::vector<StorageInsight> GetCriticalInsights();
    void AcknowledgeInsight(size_t insight_index);
    
    // Reporting and visualization
    std::string GenerateTextReport(const std::string& report_type = "summary");
    std::string GenerateJsonReport();
    std::unordered_map<std::string, std::vector<double>> GetVisualizationData(const std::string& chart_type);
    
    // Configuration
    void SetAnalyticsConfig(const AnalyticsConfig& config);
    AnalyticsConfig GetAnalyticsConfig() const;
    void SetPerformanceThresholds(const PerformanceThresholds& thresholds);
    PerformanceThresholds GetPerformanceThresholds() const;
    
    // Statistics
    size_t GetTotalFilesMonitored();
    size_t GetTotalMetricsCollected();
    std::chrono::system_clock::time_point GetOldestDataPoint();
    double GetDataRetentionEfficiency();

private:
    // Background threads
    void MonitoringLoop();
    void AnalysisLoop();
    void PredictionLoop();
    
    // Analysis algorithms
    void AnalyzeUsagePatterns();
    void DetectAnomalies();
    void IdentifyOptimizationOpportunities();
    void UpdatePredictionModels();
    
    // Machine learning implementations
    double LinearRegression(const std::vector<std::pair<double, double>>& data, double input);
    std::vector<double> NeuralNetworkPredict(const std::vector<double>& inputs, const PredictionModel& model);
    double RandomForestPredict(const std::vector<double>& features, const PredictionModel& model);
    std::vector<double> QuantumEnhancedPredict(const std::vector<double>& inputs);
    std::vector<double> EnsemblePredict(const std::vector<double>& inputs);
    
    // Pattern recognition
    std::string ClassifyAccessPattern(const std::vector<std::chrono::system_clock::time_point>& access_times);
    double CalculateSeasonality(const std::vector<PerformanceMetric>& metrics);
    std::vector<double> ExtractFeatures(const UsagePattern& pattern);
    
    // Insight generation algorithms
    void GenerateStorageEfficiencyInsights();
    void GenerateCompressionInsights();
    void GenerateCacheOptimizationInsights();
    void GenerateCapacityPlanningInsights();
    void GenerateSecurityInsights();
    
    // Utility functions
    double CalculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);
    std::vector<double> ApplyMovingAverage(const std::vector<double>& data, int window_size);
    double CalculateStandardDeviation(const std::vector<double>& data);
    bool IsAnomaly(double value, const std::vector<double>& historical_data, double sensitivity = 2.0);
    
    // Data management
    void CleanupOldData();
    void CompressHistoricalData();
    void ExportDataForTraining();
    
    // Quantum simulation for predictions
    std::vector<double> ApplyQuantumInterference(const std::vector<double>& classical_predictions);
    double CalculateQuantumProbability(const std::vector<double>& quantum_states);
    void UpdateQuantumCoherence();
    
    // Report formatting
    std::string FormatMetricValue(double value, const std::string& unit);
    std::string FormatTimestamp(std::chrono::system_clock::time_point timestamp);
    std::string FormatDuration(std::chrono::duration<double> duration);
};

} // namespace StorageOpt