#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace StorageOpt {

// Forward declarations
class QuantumStorageSystem;

struct BatchOperation {
    enum class Type {
        CREATE_FILE,
        WRITE_FILE,
        READ_FILE,
        DELETE_FILE,
        COPY_FILE,
        MOVE_FILE
    };
    
    Type type;
    std::string path;
    std::string destination_path; // For copy/move operations
    std::vector<uint8_t> data;
    size_t virtual_size;
    bool completed;
    bool success;
    std::string error_message;
    int operation_id;
};

struct BatchResult {
    int total_operations;
    int successful_operations;
    int failed_operations;
    std::vector<BatchOperation> operations;
    double execution_time_ms;
    std::vector<std::string> error_messages;
};

using ProgressCallback = std::function<void(int completed, int total, const std::string& current_file)>;

class BatchOperationManager {
private:
    QuantumStorageSystem* storage_system_;
    
    // Thread pool for parallel operations
    std::vector<std::thread> worker_threads_;
    std::queue<BatchOperation> operation_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_;
    std::atomic<int> active_workers_;
    
    // Results tracking
    std::vector<BatchOperation> completed_operations_;
    std::mutex results_mutex_;
    
    // Progress tracking
    std::atomic<int> operations_completed_;
    std::atomic<int> operations_total_;
    ProgressCallback progress_callback_;
    std::mutex callback_mutex_;
    
    size_t max_workers_;
    static constexpr size_t DEFAULT_MAX_WORKERS = 4;

public:
    BatchOperationManager(QuantumStorageSystem* system);
    ~BatchOperationManager();
    
    // Initialization
    bool Initialize(size_t max_workers = DEFAULT_MAX_WORKERS);
    void Shutdown();
    
    // Batch operations
    BatchResult ExecuteBatch(const std::vector<BatchOperation>& operations, bool parallel = true);
    
    // Convenience methods for creating batch operations
    BatchResult CreateFiles(const std::vector<std::pair<std::string, size_t>>& files);
    BatchResult WriteFiles(const std::vector<std::tuple<std::string, const void*, size_t>>& writes);
    BatchResult DeleteFiles(const std::vector<std::string>& file_paths);
    BatchResult CopyFiles(const std::vector<std::pair<std::string, std::string>>& source_dest_pairs);
    BatchResult MoveFiles(const std::vector<std::pair<std::string, std::string>>& source_dest_pairs);
    
    // Progress callback
    void SetProgressCallback(ProgressCallback callback);
    void ClearProgressCallback();
    
    // Statistics
    size_t GetQueuedOperationsCount();
    size_t GetCompletedOperationsCount();
    double GetAverageOperationTime();
    
private:
    // Worker thread function
    void WorkerThread();
    
    // Execute individual operations
    bool ExecuteOperation(BatchOperation& operation);
    
    // Helper methods
    void NotifyProgress(const std::string& current_file);
    void ClearCompletedOperations();
};

} // namespace StorageOpt
