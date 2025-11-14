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
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable)
endif()

# Include directories
include_directories(src)

# Source files
set(CORE_SOURCES
    src/core/ml_storage_optimizer.cpp
    src/core/virtual_storage_manager.cpp
    src/core/advanced_compression_system.cpp
    src/core/usb_device_driver.cpp
    src/core/encryption_manager.cpp
    src/core/batch_operation_manager.cpp
    src/core/health_monitor.cpp
    src/core/performance_profiler.cpp
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

# API GUI support (Pure C 5D Renderer)
option(USE_IMGUI "Enable API GUI support" ON)
option(USE_SYSTEM_GLFW "Use system GLFW instead of bundled" OFF)

if(USE_IMGUI)
    # Use pure C API GUI system
    set(GUI_SOURCES
        src/gui/api_gui.c
        src/gui/gui_interface.cpp
    )
    
    # Find OpenGL
    find_package(OpenGL REQUIRED)
    
    # Handle GLFW - try bundled first, then system, then fetch
    if(NOT USE_SYSTEM_GLFW AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glfw/glfw-master/CMakeLists.txt")
        message(STATUS "Using bundled GLFW from glfw/glfw-master")
        add_subdirectory(glfw/glfw-master EXCLUDE_FROM_ALL)
        set(GLFW_TARGET glfw)
    else()
        # Try to find system GLFW
        find_package(glfw3 QUIET)
        if(glfw3_FOUND)
            message(STATUS "Using system GLFW")
            set(GLFW_TARGET glfw)
        else()
            # Fetch GLFW as last resort
            message(STATUS "Downloading GLFW via FetchContent")
            include(FetchContent)
            FetchContent_Declare(
                glfw
                GIT_REPOSITORY https://github.com/glfw/glfw.git
                GIT_TAG 3.3.8
            )
            set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(glfw)
            set(GLFW_TARGET glfw)
        endif()
    endif()
    
    target_sources(${PROJECT_NAME} PRIVATE ${GUI_SOURCES})
    target_link_libraries(${PROJECT_NAME} ${GLFW_TARGET} OpenGL::GL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_IMGUI)
endif()

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
install(FILES README.md CONTRIBUTING.md LICENSE THIRD-PARTY-LICENSES.md
    DESTINATION share/doc/${PROJECT_NAME}
)

# Install documentation directory
install(DIRECTORY docs/
    DESTINATION share/doc/${PROJECT_NAME}/docs
    OPTIONAL
)

# Create example data directory
install(DIRECTORY DESTINATION share/${PROJECT_NAME}/examples)

# Install GLFW license if using bundled version
if(USE_IMGUI AND NOT USE_SYSTEM_GLFW AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glfw/glfw-master/LICENSE.md")
    install(FILES glfw/glfw-master/LICENSE.md
        DESTINATION share/doc/${PROJECT_NAME}/third-party-licenses
        RENAME GLFW-LICENSE.md
    )
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "QuantumStorageSystem")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced ML-Powered Storage Solution with Quantum Optimization")
set(CPACK_PACKAGE_DESCRIPTION "A revolutionary storage solution that combines Machine Learning, Quantum-Inspired Algorithms, and Advanced Compression to multiply storage space beyond physical limitations.")
set(CPACK_PACKAGE_VENDOR "Quantum Storage Technologies")
set(CPACK_PACKAGE_CONTACT "support@quantumstorage.tech")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Package dependencies note
if(USE_IMGUI)
    set(CPACK_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}\n\nThis package includes bundled dependencies:\n- GLFW (included)\n- OpenGL support (requires system OpenGL drivers)")
endif()

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Quantum Storage System")
    set(CPACK_NSIS_HELP_LINK "https://github.com/kamer1337/quantum-storage-system")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/kamer1337/quantum-storage-system")
    set(CPACK_NSIS_CONTACT "support@quantumstorage.tech")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "Quantum Storage System")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    
    # DEB package specific settings
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Quantum Storage Technologies <support@quantumstorage.tech>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    if(USE_IMGUI)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libgl1")
    else()
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
    endif()
    
    # RPM package specific settings
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
    if(USE_IMGUI)
        set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++, mesa-libGL")
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++")
    endif()
endif()

include(CPack)