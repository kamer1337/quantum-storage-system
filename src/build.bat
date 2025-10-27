@echo off
REM Build script for Quantum Storage System

echo Building Quantum Storage System...
echo.

REM Check if we're in the right directory
if not exist "main.cpp" (
    echo Error: main.cpp not found. Please run this script from the src directory.
    pause
    exit /b 1
)

REM Method 1: Try using CMake (if available)
where cmake >nul 2>&1
if %errorlevel% == 0 (
    echo Found CMake, using CMake build...
    if not exist "build" mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    if %errorlevel% == 0 (
        cmake --build . --config Release
        if %errorlevel% == 0 (
            echo.
            echo Build successful! Executable is in build/bin/
            echo.
            pause
            exit /b 0
        )
    )
    cd ..
    echo CMake build failed, trying direct compilation...
    echo.
)

REM Method 2: Try using g++ (MinGW/MSYS2)
where g++ >nul 2>&1
if %errorlevel% == 0 (
    echo Found g++, compiling directly...
    g++ -std=c++17 -O2 -Wall -Wextra ^
        main.cpp ^
        quantum_storage_system.cpp ^
        core/advanced_compression_system.cpp ^
        core/ml_storage_optimizer.cpp ^
        core/virtual_storage_manager.cpp ^
        analytics/storage_analytics_dashboard.cpp ^
        cloud/cloud_storage_integration.cpp ^
        -o quantum_storage.exe
    if %errorlevel% == 0 (
        echo.
        echo Build successful! Executable: quantum_storage.exe
        echo.
        pause
        exit /b 0
    ) else (
        echo g++ compilation failed.
    )
)

REM Method 3: Try using clang++
where clang++ >nul 2>&1
if %errorlevel% == 0 (
    echo Found clang++, compiling directly...
    clang++ -std=c++17 -O2 -Wall -Wextra ^
        main.cpp ^
        quantum_storage_system.cpp ^
        core/advanced_compression_system.cpp ^
        core/ml_storage_optimizer.cpp ^
        core/virtual_storage_manager.cpp ^
        analytics/storage_analytics_dashboard.cpp ^
        cloud/cloud_storage_integration.cpp ^
        -o quantum_storage.exe
    if %errorlevel% == 0 (
        echo.
        echo Build successful! Executable: quantum_storage.exe
        echo.
        pause
        exit /b 0
    ) else (
        echo clang++ compilation failed.
    )
)

REM Method 4: Try using MSVC (cl.exe)
REM First try to find and setup Visual Studio environment
for %%i in (2022 2019 2017) do (
    if exist "C:\Program Files\Microsoft Visual Studio\%%i\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%i\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
        goto :compile_msvc
    )
    if exist "C:\Program Files\Microsoft Visual Studio\%%i\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%i\Professional\VC\Auxiliary\Build\vcvars64.bat"
        goto :compile_msvc
    )
    if exist "C:\Program Files\Microsoft Visual Studio\%%i\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\%%i\Community\VC\Auxiliary\Build\vcvars64.bat"
        goto :compile_msvc
    )
)

echo.
echo ERROR: No C++ compiler found!
echo.
echo Please install one of the following:
echo   1. Visual Studio 2017/2019/2022 (Community edition is free)
echo   2. MinGW-w64 or MSYS2
echo   3. LLVM/Clang
echo   4. CMake (recommended)
echo.
echo After installation, run this script again.
echo.
pause
exit /b 1

:compile_msvc
echo Found Visual Studio, compiling with MSVC...
cl /std:c++17 /EHsc /O2 ^
    main.cpp ^
    quantum_storage_system.cpp ^
    core/advanced_compression_system.cpp ^
    core/ml_storage_optimizer.cpp ^
    core/virtual_storage_manager.cpp ^
    analytics/storage_analytics_dashboard.cpp ^
    cloud/cloud_storage_integration.cpp ^
    /Fe:quantum_storage.exe
if %errorlevel% == 0 (
    echo.
    echo Build successful! Executable: quantum_storage.exe
    echo.
    del *.obj >nul 2>&1
) else (
    echo MSVC compilation failed.
    pause
    exit /b 1
)

pause