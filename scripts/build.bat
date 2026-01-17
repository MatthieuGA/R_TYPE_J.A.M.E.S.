@echo off
REM R-TYPE Build Script for Windows
REM Supports both vcpkg and Conan package managers
REM Requires: CMake, MSVC (Visual Studio 2019+)

setlocal EnableDelayedExpansion

echo ==========================================
echo R-TYPE Build Script (vcpkg / Conan)
echo ==========================================
echo.

REM ===========================
REM Configuration Options
REM ===========================
REM Force a specific package manager: "vcpkg", "conan", or "auto" (default)
if "%FORCE_PACKAGE_MANAGER%"=="" set "FORCE_PACKAGE_MANAGER=auto"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

REM ===========================
REM Check Prerequisites
REM ===========================

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)
echo ✓ CMake found
echo.

REM ===========================
REM Package Manager Detection
REM ===========================

set "PACKAGE_MANAGER="
set "VCPKG_TOOLCHAIN="
set "CONAN_AVAILABLE=0"

REM Check for vcpkg
if exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=vcpkg\scripts\buildsystems\vcpkg.cmake"
    echo ✓ Found local vcpkg in project directory
) else if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
        echo ✓ Found vcpkg from VCPKG_ROOT: %VCPKG_ROOT%
    )
) else if exist "..\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=..\vcpkg\scripts\buildsystems\vcpkg.cmake"
    echo ✓ Found vcpkg in parent directory
)

REM Check for Conan
where conan >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set "CONAN_AVAILABLE=1"
    echo ✓ Conan found
)

echo.

REM ===========================
REM Select Package Manager
REM ===========================

if "%FORCE_PACKAGE_MANAGER%"=="vcpkg" (
    echo Forced package manager: vcpkg
    if not defined VCPKG_TOOLCHAIN (
        echo ERROR: vcpkg forced but not found
        echo Please set VCPKG_ROOT or install vcpkg in project directory
        exit /b 1
    )
    set "PACKAGE_MANAGER=vcpkg"
    goto :build
)

if "%FORCE_PACKAGE_MANAGER%"=="conan" (
    echo Forced package manager: Conan
    if "%CONAN_AVAILABLE%"=="0" (
        echo Conan not found, attempting to install...
        call :installConan
        if %ERRORLEVEL% NEQ 0 exit /b 1
    )
    set "PACKAGE_MANAGER=conan"
    goto :build
)

REM Auto-detect: prefer vcpkg, fallback to Conan
echo Auto-detecting package manager...
echo.

if defined VCPKG_TOOLCHAIN (
    set "PACKAGE_MANAGER=vcpkg"
    goto :build
)

if "%CONAN_AVAILABLE%"=="1" (
    echo ⚠ vcpkg not found, using Conan instead
    set "PACKAGE_MANAGER=conan"
    goto :build
)

REM Neither found - prompt user
echo Neither vcpkg nor Conan found.
echo.
echo Which package manager would you like to use?
echo   1^) vcpkg (recommended^)
echo   2^) Conan
echo   3^) Cancel
echo.
set /p choice="Enter choice [1-3]: "

if "%choice%"=="1" (
    call :installVcpkg
    if %ERRORLEVEL% NEQ 0 exit /b 1
    set "PACKAGE_MANAGER=vcpkg"
    goto :build
)

if "%choice%"=="2" (
    call :installConan
    if %ERRORLEVEL% NEQ 0 exit /b 1
    set "PACKAGE_MANAGER=conan"
    goto :build
)

echo Build cancelled.
exit /b 0

REM ===========================
REM Build Section
REM ===========================
:build
echo.
echo Using package manager: %PACKAGE_MANAGER%
echo.

if "%PACKAGE_MANAGER%"=="vcpkg" (
    call :buildWithVcpkg
) else if "%PACKAGE_MANAGER%"=="conan" (
    call :buildWithConan
)

if %ERRORLEVEL% NEQ 0 exit /b 1

call :printSuccess
exit /b 0

REM ===========================
REM Functions
REM ===========================

:installVcpkg
echo.
echo Cloning vcpkg...
git clone https://github.com/microsoft/vcpkg.git
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to clone vcpkg
    exit /b 1
)

echo Bootstrapping vcpkg...
cd vcpkg
call bootstrap-vcpkg.bat
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to bootstrap vcpkg
    cd ..
    exit /b 1
)
cd ..

set "VCPKG_TOOLCHAIN=vcpkg\scripts\buildsystems\vcpkg.cmake"
echo ✓ vcpkg installed successfully
exit /b 0

:installConan
echo.
echo Installing Conan via pip...
where pip >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    pip install conan
) else (
    where pip3 >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        pip3 install conan
    ) else (
        echo ERROR: pip not found. Please install Python and pip first.
        exit /b 1
    )
)

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to install Conan
    exit /b 1
)

echo Detecting Conan profile...
conan profile detect --force 2>nul
echo ✓ Conan installed successfully
exit /b 0

:buildWithVcpkg
echo ==========================================
echo Building with vcpkg
echo ==========================================
echo.

if not exist "build" mkdir build
cd build

REM Check if CMake configuration exists
if not exist "CMakeCache.txt" (
    echo Configuring with CMake...
    cmake .. ^
        -DCMAKE_TOOLCHAIN_FILE="..\%VCPKG_TOOLCHAIN%" ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -G "Visual Studio 17 2022"

    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake configuration failed
        cd ..
        exit /b 1
    )
) else (
    echo ✓ CMake already configured (delete build\CMakeCache.txt to reconfigure^)
)

echo.
call :doBuild
cd ..
exit /b %ERRORLEVEL%

:buildWithConan
echo ==========================================
echo Building with Conan
echo ==========================================
echo.

echo Installing dependencies with Conan...
conan install . ^
    --output-folder=build ^
    --build=missing ^
    -s build_type=%BUILD_TYPE% ^
    -s compiler.cppstd=20

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Conan install failed
    exit /b 1
)

echo ✓ Conan dependencies installed
echo.

if not exist "build" mkdir build
cd build

echo Configuring with CMake (Conan)...
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -G "Visual Studio 17 2022"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
call :doBuild
cd ..
exit /b %ERRORLEVEL%

:doBuild
echo ==========================================
echo Building...
echo ==========================================
echo.

REM Try to find MSBuild
set "MSBUILD_PATH="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)

REM Build with MSBuild if found, otherwise use cmake
if defined MSBUILD_PATH (
    if exist "RTypeProject.sln" (
        echo Using MSBuild for faster incremental builds...
        "%MSBUILD_PATH%" RTypeProject.sln /p:Configuration=%BUILD_TYPE% /m /nologo /verbosity:minimal
    ) else (
        echo Using cmake to build the project...
        cmake --build . --config %BUILD_TYPE% -j
    )
) else (
    echo Using cmake to build the project...
    cmake --build . --config %BUILD_TYPE% -j
)

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
exit /b 0

:printSuccess
echo.
echo ==========================================
echo ✓ Build completed successfully!
echo ==========================================
echo.
echo To run the client:
echo   .\build\client\%BUILD_TYPE%\r-type_client.exe
echo.
echo To run the server:
echo   .\build\server\%BUILD_TYPE%\r-type_server.exe
echo.
exit /b 0

endlocal
