@echo off
echo =========================================
echo    Quantum Storage System Builder
echo =========================================
echo.

REM Check if CMake is available
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found! Please install CMake 3.15 or higher.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Check for C++ compiler
where cl >nul 2>&1
if %errorlevel% neq 0 (
    where g++ >nul 2>&1
    if %errorlevel% neq 0 (
        echo ERROR: No C++ compiler found!
        echo Please install Visual Studio 2019+ or MinGW-w64
        pause
        exit /b 1
    ) else (
        echo Found: GCC compiler
        set COMPILER=MinGW
    )
) else (
    echo Found: MSVC compiler
    set COMPILER=MSVC
)

echo.
echo Creating build directory...
if not exist build mkdir build
cd build

echo.
echo Configuring project...
if "%COMPILER%"=="MinGW" (
    cmake .. -G "MinGW Makefiles"
) else (
    cmake .. -G "Visual Studio 16 2019" -A x64
)

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo =========================================
echo    BUILD SUCCESSFUL!
echo =========================================
echo.
echo The Quantum Storage System has been built successfully!
echo.
echo To run the system:
echo   1. Navigate to the build directory
echo   2. Run: QuantumStorageSystem.exe (or Release\QuantumStorageSystem.exe)
echo.
echo Features included:
echo   - Machine Learning Storage Optimization
echo   - Quantum Space Multiplication
echo   - Advanced Compression Algorithms
echo   - Real-time Analytics Dashboard
echo   - Multi-Cloud Storage Integration
echo.

if exist Release\QuantumStorageSystem.exe (
    set EXECUTABLE=Release\QuantumStorageSystem.exe
) else if exist QuantumStorageSystem.exe (
    set EXECUTABLE=QuantumStorageSystem.exe
) else (
    echo Warning: Executable not found in expected location
    goto :end
)

echo Would you like to run the Quantum Storage System now? (Y/N)
set /p choice=
if /i "%choice%"=="Y" (
    echo.
    echo Starting Quantum Storage System...
    echo.
    %EXECUTABLE%
)

:end
echo.
echo Thank you for using Quantum Storage System!
pause