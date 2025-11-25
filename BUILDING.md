# R-TYPE Build Instructions

## What Fixed the Build

The key issue was that vcpkg was trying to use the `x64-windows` triplet (which requires Visual Studio/MSVC) instead of `x64-mingw-dynamic` (which uses MinGW/GCC).

**The solution required:**
1. Setting `VCPKG_DEFAULT_TRIPLET=x64-mingw-dynamic` environment variable
2. Passing `-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic` to CMake
3. Passing `-DVCPKG_HOST_TRIPLET=x64-mingw-dynamic` to CMake
4. Specifying MinGW Makefiles generator and the mingw32-make path

## Prerequisites

1. **CMake** (3.17+): `winget install Kitware.CMake`
2. **MinGW-w64** with GCC: Download from https://github.com/niXman/mingw-builds-binaries/releases
3. **Git** (for vcpkg submodule)

## Quick Build (Windows)

The `build.bat` script is now properly configured. Just run:

```cmd
.\build.bat
```

**Note:** If your MinGW is installed in a different location than `D:\mingw64\bin`, edit the `MINGW_PATH` variable at the top of `build.bat`.

## Manual Build (Windows PowerShell)

If you want to build manually:

```powershell
# Set environment
$env:PATH += ";C:\Program Files\CMake\bin;D:\mingw64\bin"
$env:VCPKG_DEFAULT_TRIPLET = "x64-mingw-dynamic"

# Configure
mkdir build
cd build
cmake .. -G "MinGW Makefiles" `
    -DCMAKE_MAKE_PROGRAM="D:\mingw64\bin\mingw32-make.exe" `
    -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release `
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
    -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic

# Build
cmake --build . --config Release

# Run
cd ..
.\r-type_client.exe
```

## Linux Build

For Linux, the process is simpler:

```bash
chmod +x build.sh
./build.sh
```

## First Build Time

**Warning:** The first build will take 10-20 minutes because vcpkg needs to compile SFML and all its dependencies (freetype, libpng, zlib, etc.) from source with MinGW.

Subsequent builds will be much faster as vcpkg caches the compiled libraries.

## Troubleshooting

### "CMake is not installed or not in PATH"
- Install CMake and restart your terminal/IDE
- Or temporarily add to PATH: `$env:PATH += ";C:\Program Files\CMake\bin"`

### "MinGW g++ not found"
- Install MinGW-w64
- Edit `MINGW_PATH` in `build.bat` to match your installation
- Or temporarily add to PATH: `$env:PATH += ";D:\mingw64\bin"`

### "Unable to find a valid Visual Studio instance"
- This means vcpkg is trying to use the wrong triplet
- Make sure `VCPKG_DEFAULT_TRIPLET=x64-mingw-dynamic` is set
- Make sure both `VCPKG_TARGET_TRIPLET` and `VCPKG_HOST_TRIPLET` are set to `x64-mingw-dynamic`

### Clean Build
```powershell
rm -r -force build
.\build.bat
```
