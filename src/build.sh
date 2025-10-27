#!/bin/bash
# Build script for Quantum Storage System (Linux/macOS)

echo "Building Quantum Storage System..."
echo

# Check if we're in the right directory
if [ ! -f "main.cpp" ]; then
    echo "Error: main.cpp not found. Please run this script from the src directory."
    exit 1
fi

# Method 1: Try using CMake (if available)
if command -v cmake >/dev/null 2>&1; then
    echo "Found CMake, using CMake build..."
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    if [ $? -eq 0 ]; then
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        if [ $? -eq 0 ]; then
            echo
            echo "Build successful! Executable is in build/bin/"
            echo
            exit 0
        fi
    fi
    cd ..
    echo "CMake build failed, trying direct compilation..."
    echo
fi

# Method 2: Try using g++
if command -v g++ >/dev/null 2>&1; then
    echo "Found g++, compiling directly..."
    g++ -std=c++17 -O2 -Wall -Wextra -pthread \
        main.cpp \
        quantum_storage_system.cpp \
        core/advanced_compression_system.cpp \
        core/ml_storage_optimizer.cpp \
        core/virtual_storage_manager.cpp \
        analytics/storage_analytics_dashboard.cpp \
        cloud/cloud_storage_integration.cpp \
        -o quantum_storage
    if [ $? -eq 0 ]; then
        echo
        echo "Build successful! Executable: quantum_storage"
        echo
        exit 0
    else
        echo "g++ compilation failed."
    fi
fi

# Method 3: Try using clang++
if command -v clang++ >/dev/null 2>&1; then
    echo "Found clang++, compiling directly..."
    clang++ -std=c++17 -O2 -Wall -Wextra -pthread \
        main.cpp \
        quantum_storage_system.cpp \
        core/advanced_compression_system.cpp \
        core/ml_storage_optimizer.cpp \
        core/virtual_storage_manager.cpp \
        analytics/storage_analytics_dashboard.cpp \
        cloud/cloud_storage_integration.cpp \
        -o quantum_storage
    if [ $? -eq 0 ]; then
        echo
        echo "Build successful! Executable: quantum_storage"
        echo
        exit 0
    else
        echo "clang++ compilation failed."
    fi
fi

echo
echo "ERROR: No C++ compiler found!"
echo
echo "Please install one of the following:"
echo "  1. GCC (g++)"
echo "  2. Clang (clang++)"
echo "  3. CMake (recommended)"
echo
echo "On Ubuntu/Debian: sudo apt install build-essential cmake"
echo "On CentOS/RHEL: sudo yum install gcc-c++ make cmake"
echo "On macOS: xcode-select --install"
echo
echo "After installation, run this script again."
echo
exit 1