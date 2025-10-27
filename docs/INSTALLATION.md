# Quantum Storage System - Installation Guide

## Prerequisites

### System Requirements
- **OS**: Windows 10/11, Linux (Ubuntu 20.04+, Fedora 33+), macOS 11+
- **RAM**: Minimum 4GB (8GB recommended for development)
- **Disk Space**: At least 2GB for source and build

### Compiler Requirements
- **GCC**: Version 8 or higher (Linux)
- **Clang**: Version 7 or higher (macOS, Linux)
- **MSVC**: Visual Studio 2019 or higher (Windows)
- **C++ Standard**: C++17 support required

### Build Tools
- **CMake**: Version 3.15 or higher
- **Git**: For cloning the repository

## Installation Steps

### 1. Clone the Repository

```bash
git clone https://github.com/your-org/quantum-storage-system.git
cd quantum-storage-system
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure with CMake

#### Windows (MSVC)
```bash
cmake .. -G "Visual Studio 16 2019" -A x64
```

#### Linux (GCC)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### macOS (Clang)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 4. Build the Project

#### Windows
```bash
cmake --build . --config Release
```

#### Linux/macOS
```bash
cmake --build . --config Release
```

Or with make:
```bash
make -j$(nproc)
```

### 5. Verify Installation

```bash
# Run tests
ctest --verbose

# Run the demo
./Release/quantum_storage
# or on Linux/macOS
./Release/quantum_storage
```

## Configuration

### Configuration File

Create `config/quantum_storage_config.json`:

```json
{
  "physical_limit_gb": 5,
  "quantum_multiplier_max": 10.0,
  "ml_optimization_enabled": true,
  "cloud_integration_enabled": true,
  "analytics_retention_days": 30,
  "quantum_parameters": {
    "entanglement_threshold": 0.5,
    "coherence_time_hours": 24.0,
    "superposition_iterations": 100,
    "quantum_compression_enabled": true
  },
  "cloud_providers": [
    {
      "provider": "AZURE_BLOB",
      "enabled": true,
      "account_name": "your_account",
      "region": "eastus"
    },
    {
      "provider": "AWS_S3",
      "enabled": true,
      "region": "us-east-1"
    }
  ]
}
```

### Environment Variables

```bash
# Linux/macOS
export QUANTUM_STORAGE_CONFIG=./config/quantum_storage_config.json
export QUANTUM_STORAGE_LOG_LEVEL=INFO

# Windows (PowerShell)
$env:QUANTUM_STORAGE_CONFIG = ".\config\quantum_storage_config.json"
$env:QUANTUM_STORAGE_LOG_LEVEL = "INFO"
```

## Development Setup

### Recommended IDE Configuration

#### Visual Studio Code
Create `.vscode/settings.json`:
```json
{
  "C_Cpp.default.configurationName": "Release",
  "C_Cpp.default.includePath": [
    "${workspaceFolder}/include",
    "${workspaceFolder}/src"
  ],
  "editor.formatOnSave": true,
  "editor.defaultFormatter": "ms-vscode.cpptools"
}
```

#### Visual Studio
- Open `quantum-storage-system.sln` in `build/` directory
- Select `Release` configuration
- Build → Build Solution

#### CLion
- Open project directory
- CMake will auto-detect
- Build → Build Project

### Code Formatting

Install clang-format:
```bash
# Ubuntu/Debian
sudo apt-get install clang-format

# macOS
brew install clang-format

# Windows (via Chocolatey)
choco install llvm
```

Format code:
```bash
clang-format -i src/**/*.cpp include/**/*.h
```

## Troubleshooting

### CMake Configuration Issues

**Error**: `CMake Error at CMakeLists.txt:1 (cmake_minimum_required)`
- **Solution**: Update CMake to version 3.15+
  ```bash
  cmake --version
  ```

**Error**: `C++ compiler not found`
- **Solution**: Install compiler
  - Windows: Install Visual Studio 2019 or higher
  - Linux: `sudo apt-get install g++ build-essential`
  - macOS: `xcode-select --install`

### Build Errors

**Error**: `fatal error: C1083: Cannot open include file`
- **Solution**: Ensure all headers are in correct paths
  ```bash
  cmake --build . --verbose
  ```

**Error**: `Linker error: unresolved external symbol`
- **Solution**: Clean and rebuild
  ```bash
  cd build
  rm -rf * (or del * on Windows)
  cmake ..
  cmake --build .
  ```

### Runtime Issues

**Error**: `Configuration file not found`
- **Solution**: Ensure `config/quantum_storage_config.json` exists
  ```bash
  cp config/quantum_storage_config.json.example config/quantum_storage_config.json
  ```

**Error**: `Failed to connect to cloud provider`
- **Solution**: Verify credentials in configuration
  ```json
  "cloud_providers": [{
    "provider": "AZURE_BLOB",
    "account_name": "your_account",
    "access_key": "your_key"
  }]
  ```

## Building Documentation

Generate HTML documentation:
```bash
cd build
cmake .. -DBUILD_DOCS=ON
cmake --build . --target docs
```

## Next Steps

1. **Read the README**: [README.md](../README.md)
2. **Review Examples**: See [examples/](../examples/) directory
3. **Check API**: Read [docs/API.md](./API.md)
4. **Run Tests**: `ctest --verbose`
5. **Review Architecture**: [docs/ARCHITECTURE.md](./ARCHITECTURE.md)

## Getting Help

- **Documentation**: Check [docs/](./README.md)
- **Issues**: [GitHub Issues](https://github.com/your-org/quantum-storage-system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-org/quantum-storage-system/discussions)
