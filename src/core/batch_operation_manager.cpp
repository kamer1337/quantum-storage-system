#include "batch_operation_manager.h"
#include "../quantum_storage_system.h"
#include <chrono>
#include <algorithm>

namespace StorageOpt {

BatchOperationManager::BatchOperationManager(QuantumStorageSystem* system)
    : storage_system_(system)
    , running_(false)
    , active_workers_(0)
    , operations_completed_(0)
    , operations_total_(0)
    , max_workers_(DEFAULT_MAX_WORKERS) {
}

BatchOperationManager::~BatchOperationManager() {
    Shutdown();
}

bool BatchOperationManager::Initialize(size_t max_workers) {
    max_workers_ = max_workers > 0 ? max_workers : DEFAULT_MAX_WORKERS;
    running_ = true;
    
    // Start worker threads
    for (size_t i = 0; i < max_workers_; ++i) {
        worker_threads_.emplace_back(&BatchOperationManager::WorkerThread, this);
    }
    
    return true;
}

void BatchOperationManager::Shutdown() {
    running_ = false;
    queue_cv_.notify_all();
    
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
    ClearCompletedOperations();
}

BatchResult BatchOperationManager::ExecuteBatch(const std::vector<BatchOperation>& operations, bool parallel) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    operations_total_ = static_cast<int>(operations.size());
    operations_completed_ = 0;
    
    ClearCompletedOperations();
    
    if (parallel && running_) {
        // Queue operations for parallel execution
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            for (auto op : operations) {
                operation_queue_.push(op);
            }
        }
        queue_cv_.notify_all();
        
        // Wait for all operations to complete
        while (operations_completed_ < operations_total_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else {
        // Execute sequentially
        for (auto op : operations) {
            BatchOperation operation = op;
            ExecuteOperation(operation);
            
            std::lock_guard<std::mutex> lock(results_mutex_);
            completed_operations_.push_back(operation);
            operations_completed_++;
            NotifyProgress(operation.path);
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    double execution_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    // Collect results
    std::lock_guard<std::mutex> lock(results_mutex_);
    BatchResult result;
    result.total_operations = static_cast<int>(completed_operations_.size());
    result.successful_operations = 0;
    result.failed_operations = 0;
    result.operations = completed_operations_;
    result.execution_time_ms = execution_time;
    
    for (const auto& op : completed_operations_) {
        if (op.success) {
            result.successful_operations++;
        } else {
            result.failed_operations++;
            result.error_messages.push_back(op.path + ": " + op.error_message);
        }
    }
    
    return result;
}

BatchResult BatchOperationManager::CreateFiles(const std::vector<std::pair<std::string, size_t>>& files) {
    std::vector<BatchOperation> operations;
    int op_id = 0;
    
    for (const auto& file : files) {
        BatchOperation op;
        op.type = BatchOperation::Type::CREATE_FILE;
        op.path = file.first;
        op.virtual_size = file.second;
        op.operation_id = op_id++;
        op.completed = false;
        operations.push_back(op);
    }
    
    return ExecuteBatch(operations, true);
}

BatchResult BatchOperationManager::WriteFiles(const std::vector<std::tuple<std::string, const void*, size_t>>& writes) {
    std::vector<BatchOperation> operations;
    int op_id = 0;
    
    for (const auto& write : writes) {
        BatchOperation op;
        op.type = BatchOperation::Type::WRITE_FILE;
        op.path = std::get<0>(write);
        
        const uint8_t* data_ptr = static_cast<const uint8_t*>(std::get<1>(write));
        size_t size = std::get<2>(write);
        op.data = std::vector<uint8_t>(data_ptr, data_ptr + size);
        
        op.operation_id = op_id++;
        op.completed = false;
        operations.push_back(op);
    }
    
    return ExecuteBatch(operations, true);
}

BatchResult BatchOperationManager::DeleteFiles(const std::vector<std::string>& file_paths) {
    std::vector<BatchOperation> operations;
    int op_id = 0;
    
    for (const auto& path : file_paths) {
        BatchOperation op;
        op.type = BatchOperation::Type::DELETE_FILE;
        op.path = path;
        op.operation_id = op_id++;
        op.completed = false;
        operations.push_back(op);
    }
    
    return ExecuteBatch(operations, true);
}

BatchResult BatchOperationManager::CopyFiles(const std::vector<std::pair<std::string, std::string>>& source_dest_pairs) {
    std::vector<BatchOperation> operations;
    int op_id = 0;
    
    for (const auto& pair : source_dest_pairs) {
        BatchOperation op;
        op.type = BatchOperation::Type::COPY_FILE;
        op.path = pair.first;
        op.destination_path = pair.second;
        op.operation_id = op_id++;
        op.completed = false;
        operations.push_back(op);
    }
    
    return ExecuteBatch(operations, true);
}

BatchResult BatchOperationManager::MoveFiles(const std::vector<std::pair<std::string, std::string>>& source_dest_pairs) {
    std::vector<BatchOperation> operations;
    int op_id = 0;
    
    for (const auto& pair : source_dest_pairs) {
        BatchOperation op;
        op.type = BatchOperation::Type::MOVE_FILE;
        op.path = pair.first;
        op.destination_path = pair.second;
        op.operation_id = op_id++;
        op.completed = false;
        operations.push_back(op);
    }
    
    return ExecuteBatch(operations, true);
}

void BatchOperationManager::SetProgressCallback(ProgressCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    progress_callback_ = callback;
}

void BatchOperationManager::ClearProgressCallback() {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    progress_callback_ = nullptr;
}

size_t BatchOperationManager::GetQueuedOperationsCount() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return operation_queue_.size();
}

size_t BatchOperationManager::GetCompletedOperationsCount() {
    std::lock_guard<std::mutex> lock(results_mutex_);
    return completed_operations_.size();
}

double BatchOperationManager::GetAverageOperationTime() {
    // This would track individual operation times in a real implementation
    return 0.0;
}

void BatchOperationManager::WorkerThread() {
    active_workers_++;
    
    while (running_) {
        BatchOperation operation;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !operation_queue_.empty() || !running_; });
            
            if (!running_ && operation_queue_.empty()) {
                break;
            }
            
            if (!operation_queue_.empty()) {
                operation = operation_queue_.front();
                operation_queue_.pop();
            } else {
                continue;
            }
        }
        
        // Execute the operation
        ExecuteOperation(operation);
        
        // Store result
        {
            std::lock_guard<std::mutex> lock(results_mutex_);
            completed_operations_.push_back(operation);
        }
        
        operations_completed_++;
        NotifyProgress(operation.path);
    }
    
    active_workers_--;
}

bool BatchOperationManager::ExecuteOperation(BatchOperation& operation) {
    if (!storage_system_) {
        operation.completed = true;
        operation.success = false;
        operation.error_message = "Storage system not initialized";
        return false;
    }
    
    bool result = false;
    
    switch (operation.type) {
        case BatchOperation::Type::CREATE_FILE:
            result = storage_system_->CreateFile(operation.path, operation.virtual_size);
            operation.error_message = result ? "" : "Failed to create file";
            break;
            
        case BatchOperation::Type::WRITE_FILE:
            result = storage_system_->WriteFile(operation.path, operation.data.data(), operation.data.size());
            operation.error_message = result ? "" : "Failed to write file";
            break;
            
        case BatchOperation::Type::READ_FILE: {
            std::vector<char> buffer(1024 * 1024); // 1MB buffer
            size_t size = buffer.size();
            result = storage_system_->ReadFile(operation.path, buffer.data(), size);
            operation.error_message = result ? "" : "Failed to read file";
            break;
        }
            
        case BatchOperation::Type::DELETE_FILE:
            result = storage_system_->DeleteFile(operation.path);
            operation.error_message = result ? "" : "Failed to delete file";
            break;
            
        case BatchOperation::Type::COPY_FILE: {
            // Read source file
            std::vector<char> buffer(10 * 1024 * 1024); // 10MB buffer
            size_t size = buffer.size();
            if (storage_system_->ReadFile(operation.path, buffer.data(), size)) {
                // Create and write destination file
                if (storage_system_->CreateFile(operation.destination_path, size)) {
                    result = storage_system_->WriteFile(operation.destination_path, buffer.data(), size);
                    operation.error_message = result ? "" : "Failed to write destination file";
                } else {
                    operation.error_message = "Failed to create destination file";
                }
            } else {
                operation.error_message = "Failed to read source file";
            }
            break;
        }
            
        case BatchOperation::Type::MOVE_FILE: {
            // Copy then delete
            std::vector<char> buffer(10 * 1024 * 1024);
            size_t size = buffer.size();
            if (storage_system_->ReadFile(operation.path, buffer.data(), size)) {
                if (storage_system_->CreateFile(operation.destination_path, size)) {
                    if (storage_system_->WriteFile(operation.destination_path, buffer.data(), size)) {
                        result = storage_system_->DeleteFile(operation.path);
                        operation.error_message = result ? "" : "Failed to delete source file";
                    } else {
                        operation.error_message = "Failed to write destination file";
                    }
                } else {
                    operation.error_message = "Failed to create destination file";
                }
            } else {
                operation.error_message = "Failed to read source file";
            }
            break;
        }
    }
    
    operation.completed = true;
    operation.success = result;
    return result;
}

void BatchOperationManager::NotifyProgress(const std::string& current_file) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (progress_callback_) {
        progress_callback_(operations_completed_, operations_total_, current_file);
    }
}

void BatchOperationManager::ClearCompletedOperations() {
    std::lock_guard<std::mutex> lock(results_mutex_);
    completed_operations_.clear();
}

} // namespace StorageOpt
