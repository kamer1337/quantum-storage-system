#include "encryption_manager.h"
#include <algorithm>
#include <random>
#include <cstring>
#include <fstream>
#include <iostream>

namespace StorageOpt {

EncryptionManager::EncryptionManager() 
    : default_algorithm_(EncryptionAlgorithm::AES_256) {
}

EncryptionManager::~EncryptionManager() {
    Shutdown();
}

bool EncryptionManager::Initialize(EncryptionAlgorithm default_algorithm) {
    default_algorithm_ = default_algorithm;
    
    // Generate a default key
    default_key_id_ = GenerateKey(default_algorithm);
    
    return !default_key_id_.empty();
}

void EncryptionManager::Shutdown() {
    std::lock_guard<std::mutex> lock(keys_mutex_);
    keys_.clear();
    default_key_id_.clear();
}

std::string EncryptionManager::GenerateKey(EncryptionAlgorithm algorithm) {
    size_t key_size = 0;
    switch (algorithm) {
        case EncryptionAlgorithm::AES_128:
            key_size = 16; // 128 bits
            break;
        case EncryptionAlgorithm::AES_256:
            key_size = 32; // 256 bits
            break;
        case EncryptionAlgorithm::CHACHA20:
            key_size = 32; // 256 bits
            break;
        case EncryptionAlgorithm::QUANTUM_RESISTANT:
            key_size = 64; // 512 bits
            break;
        default:
            return "";
    }
    
    std::vector<uint8_t> key_data = GenerateRandomBytes(key_size);
    
    // Generate unique key ID
    std::string key_id = "key_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    
    EncryptionKey key;
    key.key_data = key_data;
    key.algorithm = algorithm;
    key.key_id = key_id;
    key.created_at = std::chrono::system_clock::now();
    key.is_active = true;
    
    std::lock_guard<std::mutex> lock(keys_mutex_);
    keys_[key_id] = key;
    
    return key_id;
}

bool EncryptionManager::AddKey(const std::string& key_id, const std::vector<uint8_t>& key_data, EncryptionAlgorithm algorithm) {
    EncryptionKey key;
    key.key_data = key_data;
    key.algorithm = algorithm;
    key.key_id = key_id;
    key.created_at = std::chrono::system_clock::now();
    key.is_active = true;
    
    if (!ValidateKey(key)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(keys_mutex_);
    keys_[key_id] = key;
    return true;
}

bool EncryptionManager::RemoveKey(const std::string& key_id) {
    std::lock_guard<std::mutex> lock(keys_mutex_);
    return keys_.erase(key_id) > 0;
}

bool EncryptionManager::SetDefaultKey(const std::string& key_id) {
    std::lock_guard<std::mutex> lock(keys_mutex_);
    if (keys_.find(key_id) != keys_.end()) {
        default_key_id_ = key_id;
        return true;
    }
    return false;
}

EncryptionResult EncryptionManager::EncryptData(const std::vector<uint8_t>& data, const std::string& key_id) {
    std::string effective_key_id = key_id.empty() ? default_key_id_ : key_id;
    
    EncryptionKey key;
    {
        std::lock_guard<std::mutex> lock(keys_mutex_);
        auto it = keys_.find(effective_key_id);
        if (it == keys_.end()) {
            return {false, {}, "Key not found", 0, 0, EncryptionAlgorithm::NONE};
        }
        key = it->second;
    }
    
    switch (key.algorithm) {
        case EncryptionAlgorithm::AES_128:
        case EncryptionAlgorithm::AES_256:
            return EncryptAES(data, key);
        case EncryptionAlgorithm::CHACHA20:
            return EncryptChaCha20(data, key);
        case EncryptionAlgorithm::QUANTUM_RESISTANT:
            return EncryptQuantumResistant(data, key);
        default:
            return {false, {}, "Unsupported algorithm", 0, 0, EncryptionAlgorithm::NONE};
    }
}

EncryptionResult EncryptionManager::DecryptData(const std::vector<uint8_t>& encrypted_data, const std::string& key_id) {
    std::string effective_key_id = key_id.empty() ? default_key_id_ : key_id;
    
    EncryptionKey key;
    {
        std::lock_guard<std::mutex> lock(keys_mutex_);
        auto it = keys_.find(effective_key_id);
        if (it == keys_.end()) {
            return {false, {}, "Key not found", 0, 0, EncryptionAlgorithm::NONE};
        }
        key = it->second;
    }
    
    switch (key.algorithm) {
        case EncryptionAlgorithm::AES_128:
        case EncryptionAlgorithm::AES_256:
            return DecryptAES(encrypted_data, key);
        case EncryptionAlgorithm::CHACHA20:
            return DecryptChaCha20(encrypted_data, key);
        case EncryptionAlgorithm::QUANTUM_RESISTANT:
            return DecryptQuantumResistant(encrypted_data, key);
        default:
            return {false, {}, "Unsupported algorithm", 0, 0, EncryptionAlgorithm::NONE};
    }
}

EncryptionResult EncryptionManager::EncryptFile(const std::string& input_path, const std::string& output_path, const std::string& key_id) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input) {
        return {false, {}, "Failed to open input file", 0, 0, EncryptionAlgorithm::NONE};
    }
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();
    
    auto result = EncryptData(data, key_id);
    
    if (result.success) {
        std::ofstream output(output_path, std::ios::binary);
        if (!output) {
            return {false, {}, "Failed to open output file", 0, 0, EncryptionAlgorithm::NONE};
        }
        output.write(reinterpret_cast<const char*>(result.data.data()), result.data.size());
        output.close();
    }
    
    return result;
}

EncryptionResult EncryptionManager::DecryptFile(const std::string& input_path, const std::string& output_path, const std::string& key_id) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input) {
        return {false, {}, "Failed to open input file", 0, 0, EncryptionAlgorithm::NONE};
    }
    
    std::vector<uint8_t> encrypted_data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();
    
    auto result = DecryptData(encrypted_data, key_id);
    
    if (result.success) {
        std::ofstream output(output_path, std::ios::binary);
        if (!output) {
            return {false, {}, "Failed to open output file", 0, 0, EncryptionAlgorithm::NONE};
        }
        output.write(reinterpret_cast<const char*>(result.data.data()), result.data.size());
        output.close();
    }
    
    return result;
}

bool EncryptionManager::IsFileEncrypted(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) return false;
    
    // Read first 16 bytes to check for encryption markers
    std::vector<uint8_t> header(16, 0);
    file.read(reinterpret_cast<char*>(header.data()), 16);
    
    // Simple heuristic: check if file has high entropy (typical of encrypted data)
    // In production, would check for specific encryption headers/magic numbers
    // For now, return false as placeholder since we don't have proper detection
    return false; // Placeholder - would need proper encryption header detection
}

EncryptionAlgorithm EncryptionManager::DetectEncryptionAlgorithm(const std::vector<uint8_t>& data) {
    // In production, this would analyze file headers or metadata to detect the algorithm
    // For now, return NONE since we can't reliably detect without proper headers
    if (data.empty()) {
        return EncryptionAlgorithm::NONE;
    }
    
    // Placeholder - in production would check encryption headers/magic bytes
    return EncryptionAlgorithm::NONE; // Unknown/unable to detect
}

std::vector<std::string> EncryptionManager::GetAvailableKeys() {
    std::lock_guard<std::mutex> lock(keys_mutex_);
    std::vector<std::string> key_ids;
    for (const auto& pair : keys_) {
        if (pair.second.is_active) {
            key_ids.push_back(pair.first);
        }
    }
    return key_ids;
}

// Simplified encryption implementations (in production, use proper crypto libraries)
EncryptionResult EncryptionManager::EncryptAES(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    // Simplified XOR-based encryption for demonstration
    std::vector<uint8_t> encrypted = data;
    for (size_t i = 0; i < encrypted.size(); ++i) {
        encrypted[i] ^= key.key_data[i % key.key_data.size()];
    }
    
    return {
        true,
        encrypted,
        "",
        data.size(),
        encrypted.size(),
        key.algorithm
    };
}

EncryptionResult EncryptionManager::DecryptAES(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    // XOR is symmetric, so decryption is the same as encryption
    return EncryptAES(encrypted_data, key);
}

EncryptionResult EncryptionManager::EncryptChaCha20(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    // Simplified stream cipher simulation
    std::vector<uint8_t> encrypted = data;
    uint32_t counter = 0;
    
    for (size_t i = 0; i < encrypted.size(); ++i) {
        uint8_t keystream_byte = key.key_data[(i + counter) % key.key_data.size()];
        encrypted[i] ^= keystream_byte;
        if ((i + 1) % 64 == 0) counter++;
    }
    
    return {
        true,
        encrypted,
        "",
        data.size(),
        encrypted.size(),
        key.algorithm
    };
}

EncryptionResult EncryptionManager::DecryptChaCha20(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    // Stream cipher is symmetric
    return EncryptChaCha20(encrypted_data, key);
}

EncryptionResult EncryptionManager::EncryptQuantumResistant(const std::vector<uint8_t>& data, const EncryptionKey& key) {
    // Quantum-resistant simulation with multiple rounds
    std::vector<uint8_t> encrypted = data;
    
    // Multiple encryption rounds for added security
    for (int round = 0; round < 3; ++round) {
        for (size_t i = 0; i < encrypted.size(); ++i) {
            size_t key_idx = (i * (round + 1)) % key.key_data.size();
            encrypted[i] ^= key.key_data[key_idx];
            encrypted[i] = (encrypted[i] + key.key_data[(key_idx + 1) % key.key_data.size()]) % 256;
        }
    }
    
    return {
        true,
        encrypted,
        "",
        data.size(),
        encrypted.size(),
        key.algorithm
    };
}

EncryptionResult EncryptionManager::DecryptQuantumResistant(const std::vector<uint8_t>& encrypted_data, const EncryptionKey& key) {
    // Reverse the quantum-resistant encryption
    std::vector<uint8_t> decrypted = encrypted_data;
    
    // Reverse the rounds
    for (int round = 2; round >= 0; --round) {
        for (size_t i = 0; i < decrypted.size(); ++i) {
            size_t key_idx = (i * (round + 1)) % key.key_data.size();
            decrypted[i] = (decrypted[i] - key.key_data[(key_idx + 1) % key.key_data.size()] + 256) % 256;
            decrypted[i] ^= key.key_data[key_idx];
        }
    }
    
    return {
        true,
        decrypted,
        "",
        encrypted_data.size(),
        decrypted.size(),
        key.algorithm
    };
}

std::vector<uint8_t> EncryptionManager::GenerateRandomBytes(size_t length) {
    std::vector<uint8_t> bytes(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        bytes[i] = static_cast<uint8_t>(dis(gen));
    }
    
    return bytes;
}

std::vector<uint8_t> EncryptionManager::DeriveKey(const std::vector<uint8_t>& input, size_t output_length) {
    // Simple key derivation using repeated hashing
    std::vector<uint8_t> derived(output_length);
    for (size_t i = 0; i < output_length; ++i) {
        derived[i] = input[i % input.size()] ^ static_cast<uint8_t>(i);
    }
    return derived;
}

bool EncryptionManager::ValidateKey(const EncryptionKey& key) {
    size_t expected_size = 0;
    switch (key.algorithm) {
        case EncryptionAlgorithm::AES_128:
            expected_size = 16;
            break;
        case EncryptionAlgorithm::AES_256:
        case EncryptionAlgorithm::CHACHA20:
            expected_size = 32;
            break;
        case EncryptionAlgorithm::QUANTUM_RESISTANT:
            expected_size = 64;
            break;
        default:
            return false;
    }
    
    return key.key_data.size() == expected_size && !key.key_id.empty();
}

} // namespace StorageOpt
