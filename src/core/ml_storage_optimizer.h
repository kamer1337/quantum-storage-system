#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <filesystem>

namespace StorageOpt {

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

struct CompressionAlgorithm {
    enum Type {
        NONE = 0,
        LZ4,
        ZSTD,
        BROTLI,
        QUANTUM_INSPIRED,
        ML_ADAPTIVE
    };
    
    Type type;
    int compression_level;
    double efficiency_score;
};

class MLStorageOptimizer {
private:
    std::unordered_map<std::string, FileMetadata> file_database_;
    std::mutex db_mutex_;
    std::atomic<bool> running_;
    std::thread optimization_thread_;
    std::thread learning_thread_;
    
    // ML Model weights for file priority scoring
    struct MLWeights {
        double access_frequency_weight = 0.3;
        double file_size_weight = 0.2;
        double recency_weight = 0.25;
        double compression_potential_weight = 0.15;
        double file_type_weight = 0.1;
    } ml_weights_;
    
    // Quantum-inspired optimization parameters
    struct QuantumParams {
        double entanglement_factor = 0.7;
        double superposition_threshold = 0.5;
        int quantum_iterations = 100;
    } quantum_params_;

public:
    MLStorageOptimizer();
    ~MLStorageOptimizer();
    
    // Core ML functions
    bool Initialize(const std::string& storage_path);
    void StartOptimization();
    void StopOptimization();
    
    // File analysis and prediction
    double PredictCompressionRatio(const std::string& file_path);
    CompressionAlgorithm SelectOptimalCompression(const FileMetadata& metadata);
    double CalculateFilePriority(const FileMetadata& metadata);
    
    // Machine Learning training
    void TrainModel(const std::vector<FileMetadata>& training_data);
    void UpdateWeights(const std::vector<double>& performance_feedback);
    
    // Quantum-inspired optimization
    std::vector<std::string> QuantumFileSelection(const std::vector<FileMetadata>& candidates);
    double QuantumCompressionOptimization(const std::string& file_path);
    
    // File management
    bool AddFile(const std::string& file_path);
    bool RemoveFile(const std::string& file_path);
    FileMetadata GetFileMetadata(const std::string& file_path);
    std::vector<FileMetadata> GetTopPriorityFiles(int count);
    
    // Statistics and monitoring
    double GetStorageEfficiency();
    size_t GetTotalManagedSpace();
    size_t GetEffectiveSpace();
    double GetCompressionRatio();
    
private:
    void OptimizationLoop();
    void LearningLoop();
    void AnalyzeFilePatterns();
    void UpdateFileMetadata(const std::string& file_path);
    double CalculateCompressionPotential(const std::string& file_path);
    void ApplyQuantumOptimization();
    
    // Neural network simulation for file priority
    double NeuralNetworkPredict(const std::vector<double>& inputs);
    void BackpropagateError(const std::vector<double>& expected, const std::vector<double>& actual);
};

} // namespace StorageOpt