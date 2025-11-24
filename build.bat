@echo off
REM R-TYPE J.A.M.E.S. Build Script for Windows
REM This script sets up vcpkg and builds the project

echo ==========================================
echo R-TYPE J.A.M.E.S. Build Script
echo ==========================================
echo.

REM Check if vcpkg is installed
if "%VCPKG_ROOT%"=="" (
    echo WARNING: VCPKG_ROOT is not set.
    echo Please install vcpkg and set VCPKG_ROOT environment variable.
    echo.
    echo Quick setup:
    echo   git clone https://github.com/microsoft/vcpkg.git
    echo   cd vcpkg
    echo   .\bootstrap-vcpkg.bat
    echo   set VCPKG_ROOT=%%CD%%
    echo.
    exit /b 1
)

echo vcpkg found at: %VCPKG_ROOT%
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed. Please install CMake 3.21 or higher.
    exit /b 1
)

for /f "tokens=3" %%a in ('cmake --version ^| findstr /R "version"') do set CMAKE_VERSION=%%a
echo CMake version: %CMAKE_VERSION%
echo.

REM Create build directory
set BUILD_DIR=build
if exist "%BUILD_DIR%" (
    echo WARNING: Build directory exists. Cleaning...
    rmdir /s /q "%BUILD_DIR%"
)

mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo.
echo ==========================================
echo Configuring with CMake...
echo ==========================================
echo.

REM Configure with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

echo.
echo ==========================================
echo Building...
echo ==========================================
echo.

REM Build
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ==========================================
echo Build completed successfully!
echo ==========================================
echo.
echo To run the client:
echo   cd %BUILD_DIR%
echo   .\Release\r-type_client.exe
echo.

cd ..
