@echo off
REM R-TYPE Build Script for Windows with MinGW
REM Requires: CMake, MinGW (GCC), Git

setlocal

echo ==========================================
echo R-TYPE Build Script (vcpkg + MinGW)
echo ==========================================
echo.

REM Set MinGW and CMake paths (adjust if needed)
set "MINGW_PATH=D:\mingw64\bin"
set "CMAKE_PATH=C:\Program Files\CMake\bin"

REM Add to PATH temporarily
set "PATH=%PATH%;%CMAKE_PATH%;%MINGW_PATH%"

REM Set vcpkg to use MinGW triplet
set "VCPKG_DEFAULT_TRIPLET=x64-mingw-dynamic"

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Check for MinGW
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MinGW g++ not found
    echo Please install MinGW or adjust MINGW_PATH in this script
    exit /b 1
)

echo ✓ CMake found
echo ✓ MinGW found
echo.

REM Bootstrap vcpkg if needed
if not exist "vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    cd vcpkg
    call bootstrap-vcpkg.bat
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to bootstrap vcpkg
        cd ..
        exit /b 1
    )
    cd ..
    echo.
)

echo ✓ vcpkg is ready
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo ==========================================
echo Configuring with CMake...
echo ==========================================
echo.

cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_MAKE_PROGRAM="%MINGW_PATH%\mingw32-make.exe" ^
    -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic ^
    -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic

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
