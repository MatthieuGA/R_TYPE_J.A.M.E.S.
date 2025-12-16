@echo off
REM R-TYPE Build Script for Windows
REM Requires: CMake, MSVC (Visual Studio 2019+), vcpkg

call "%~dp0build.bat"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
echo ==========================================
echo Running R-TYPE Server...
echo ==========================================
cd /d "%~dp0build\server\Release"
if not exist "r-type_server.exe" (
    echo ERROR: r-type_server.exe not found in %CD%
    exit /b 1
)
r-type_server.exe
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: R-TYPE Server exited with an error
    cd /d "%~dp0"
    exit /b 1
)
cd /d "%~dp0"
exit /b 0