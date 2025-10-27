cmake_minimum_required(VERSION 3.15)
project(QuantumStorageSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/analytics)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cloud)

# Collect all source files
set(CORE_SOURCES
    core/advanced_compression_system.cpp
    core/ml_storage_optimizer.cpp
    core/virtual_storage_manager.cpp
)

set(ANALYTICS_SOURCES
    analytics/storage_analytics_dashboard.cpp
)

set(CLOUD_SOURCES
    cloud/cloud_storage_integration.cpp
)

set(MAIN_SOURCES
    main.cpp
    quantum_storage_system.cpp
)

# Create the executable
add_executable(QuantumStorageSystem
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${ANALYTICS_SOURCES}
    ${CLOUD_SOURCES}
)

# Link libraries (add as needed)
if(WIN32)
    # Windows specific libraries
    target_link_libraries(QuantumStorageSystem)
else()
    # Unix/Linux specific libraries
    target_link_libraries(QuantumStorageSystem pthread)
endif()

# Set executable properties
set_target_properties(QuantumStorageSystem PROPERTIES
    OUTPUT_NAME "quantum_storage"
)

# Installation
install(TARGETS QuantumStorageSystem
    RUNTIME DESTINATION bin
)