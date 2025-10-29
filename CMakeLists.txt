cmake_minimum_required(VERSION 3.15)
project(QuantumStorageSystem VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(src)

# Source files
set(CORE_SOURCES
    src/core/ml_storage_optimizer.cpp
    src/core/virtual_storage_manager.cpp
    src/core/advanced_compression_system.cpp
    src/core/usb_device_driver.cpp
)

set(ANALYTICS_SOURCES
    src/analytics/storage_analytics_dashboard.cpp
)

set(CLOUD_SOURCES
    src/cloud/cloud_storage_integration.cpp
)

set(SYSTEM_SOURCES
    src/quantum_storage_system.cpp
    src/main.cpp
)

# Create executable
add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${ANALYTICS_SOURCES}
    ${CLOUD_SOURCES}
    ${SYSTEM_SOURCES}
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

# Find and link threads
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# Optional: Find and link compression libraries if available
# find_package(PkgConfig)
# if(PkgConfig_FOUND)
#     pkg_check_modules(LZ4 liblz4)
#     pkg_check_modules(ZSTD libzstd)
#     
#     if(LZ4_FOUND)
#         target_link_libraries(${PROJECT_NAME} ${LZ4_LIBRARIES})
#         target_include_directories(${PROJECT_NAME} PRIVATE ${LZ4_INCLUDE_DIRS})
#         add_definitions(-DHAVE_LZ4)
#     endif()
#     
#     if(ZSTD_FOUND)
#         target_link_libraries(${PROJECT_NAME} ${ZSTD_LIBRARIES})
#         target_include_directories(${PROJECT_NAME} PRIVATE ${ZSTD_INCLUDE_DIRS})
#         add_definitions(-DHAVE_ZSTD)
#     endif()
# endif()

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install documentation
install(FILES README.md
    DESTINATION share/doc/${PROJECT_NAME}
)

# Create example data directory
install(DIRECTORY DESTINATION share/${PROJECT_NAME}/examples)

# CPack configuration for packaging
include(CPack)
set(CPACK_PACKAGE_NAME "QuantumStorageSystem")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced ML-Powered Storage Solution with Quantum Optimization")
set(CPACK_PACKAGE_VENDOR "Quantum Storage Technologies")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Quantum Storage System")
    set(CPACK_NSIS_HELP_LINK "https://github.com/your-org/quantum-storage-system")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/your-org/quantum-storage-system")
    set(CPACK_NSIS_CONTACT "support@quantumstorage.tech")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()