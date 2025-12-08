@echo off
REM R-TYPE Build Script for Windows
REM Requires: CMake, MSVC (Visual Studio 2019+), vcpkg

setlocal

echo ==========================================
echo R-TYPE Build Script (vcpkg + MSVC)
echo ==========================================
echo.

REM Check for VCPKG_ROOT environment variable
if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT environment variable is not set
    echo.
    echo Please set it to your vcpkg installation directory:
    echo   $env:VCPKG_ROOT = "C:\path\to\vcpkg"
    echo.
    echo Or install vcpkg:
    echo   git clone https://github.com/microsoft/vcpkg.git
    echo   cd vcpkg
    echo   .\bootstrap-vcpkg.bat
    echo   $env:VCPKG_ROOT = $PWD
    echo.
    exit /b 1
)

echo ✓ vcpkg found at: %VCPKG_ROOT%
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

echo ✓ CMake found
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo ==========================================
echo Configuring with CMake...
echo ==========================================
echo.

cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
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
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

cd ..

echo.
echo ==========================================
echo ✓ Build completed successfully!
echo ==========================================
echo.
echo To run the client:
echo   .\r-type_client.exe
echo.

endlocal
