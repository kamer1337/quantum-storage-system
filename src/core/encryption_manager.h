#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace StorageOpt {

enum class EncryptionAlgorithm {
    NONE = 0,
    AES_128,
    AES_256,
    CHACHA20,
    QUANTUM_RESISTANT
};

struct EncryptionKey {
    std::vector<uint8_t> key_data;
    EncryptionAlgorithm algorithm;
    std::string key_id;
    std::chrono::system_clock::time_point created_at;
    bool is_active;
};

struct EncryptionResult {
    bool success;
    std::vector<uint8_t> data;
    std::string error_message;
    size_t original_size;
    size_t encrypted_size;
    EncryptionAlgorithm algorithm_used;
};

class EncryptionManager {
private:
    std::unordered_map<std::string, EncryptionKey> keys_;
    std::mutex keys_mutex_;
    EncryptionAlgorithm default_algorithm_;
    std::string default_key_id_;

public:
    EncryptionManager();
    ~EncryptionManager();
    
    // Initialization
    bool Initialize(EncryptionAlgorithm default_algorithm = EncryptionAlgorithm::AES_256);
    void Shutdown();
    
    // Key management
    std::string GenerateKey(EncryptionAlgorithm algorithm);
    bool AddKey(const std::string& key_id, const std::vector<uint8_t>& key_data, EncryptionAlgorithm algorithm);
    bool RemoveKey(const std::string& key_id);
    bool SetDefaultKey(const std::string& key_id);
    std::string GetDefaultKeyId() const { return default_key_id_; }
    
    // Encryption/Decryption operations
    EncryptionResult EncryptData(const std::vector<uint8_t>& data, const std::string& key_id = "");
    EncryptionResult DecryptData(const std::vector<uint8_t>& encrypted_data, const std::string& key_id = "");
    
    EncryptionResult EncryptFile(const std::string& input_path, const std::string& output_path, const std::string& key_id = "");
    EncryptionResult DecryptFile(const std::string& input_path, const std::string& output_path, const std::string& key_id = "");
    
    // Utility functions
    bool IsFileEncrypted(const std::string& file_path);
    EncryptionAlgorithm DetectEncryptionAlgorithm(const std::vector<uint8_t>& data);
    std::vector<std::string> GetAvailableKeys();
    
private:
    // Encryption implementations
    EncryptionResult EncryptAES(const std::vector<uint8_t>& data, const EncryptionKey& key);
    EncryptionResult DecryptAES(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key);
    
    EncryptionResult EncryptChaCha20(const std::vector<uint8_t>& data, const EncryptionKey& key);
    EncryptionResult DecryptChaCha20(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key);
    
    EncryptionResult EncryptQuantumResistant(const std::vector<uint8_t>& data, const EncryptionKey& key);
    EncryptionResult DecryptQuantumResistant(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key);
    
    // Helper functions
    std::vector<uint8_t> GenerateRandomBytes(size_t length);
    std::vector<uint8_t> DeriveKey(const std::vector<uint8_t>& input, size_t output_length);
    bool ValidateKey(const EncryptionKey& key);
};

} // namespace StorageOpt
