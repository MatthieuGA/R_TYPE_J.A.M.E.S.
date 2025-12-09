@echo off
REM R-TYPE Build Script for Windows
REM Requires: CMake, MSVC (Visual Studio 2019+), vcpkg

call "%~dp0build.bat"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
echo ==========================================
echo Running R-TYPE Client...
echo ==========================================
cd /d build\client\Release
r-type_client.exe
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: R-TYPE Client exited with an error
    cd /d "%~dp0"
    exit /b 1
)
cd /d "%~dp0"
exit /b 0