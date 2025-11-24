# R_TYPE_J.A.M.E.S.

A multiplayer networked game engine implementation of the classic R-Type game, built with modern C++ and featuring a custom game engine architecture.

## 🎮 About

This project implements a multi-threaded server and graphical client for R-Type using a custom-designed game engine. The architecture demonstrates advanced C++ development techniques and follows software engineering best practices.

## 📋 Requirements

- **C++ Compiler**: GCC 7+ (Linux) or MSVC 2019+ (Windows)
- **CMake**: 3.21 or higher
- **vcpkg**: For dependency management
- **Git**: For version control

## 🛠️ Dependencies

The project uses vcpkg for cross-platform dependency management:

- **Qt5**: GUI framework (Core, Widgets)
- **Asio**: Asynchronous networking library

All dependencies are automatically managed through vcpkg manifest mode.

## 🚀 Quick Start

### 1. Install vcpkg

**Linux/macOS:**
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)
export PATH=$VCPKG_ROOT:$PATH
```

**Windows (PowerShell):**
```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = $PWD
$env:PATH += ";$env:VCPKG_ROOT"
```

### 2. Clone and Build

**Linux:**
```bash
git clone <repository-url>
cd R_TYPE_J.A.M.E.S.
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

**Windows:**
```powershell
git clone <repository-url>
cd R_TYPE_J.A.M.E.S.
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release
```

### 3. Run

**Linux:**
```bash
./r-type_client
```

**Windows:**
```powershell
.\Release\r-type_client.exe
```

### Adding Dependencies

To add new dependencies, edit `vcpkg.json` and rebuild:
```json
{
  "dependencies": [
    "new-package-name"
  ]
}
```

## 🌐 Cross-Platform Support

This project is designed to run on:
- ✅ Linux (Ubuntu 20.04+, Fedora, Arch)
- ✅ Windows (10/11 with MSVC)


## 👥 Authors

- Development Team: J.A.M.E.S.

## 🔗 Useful Links

- [Project Documentation](docs/)
- [vcpkg Documentation](https://vcpkg.io/)
- [CMake Documentation](https://cmake.org/documentation/)

## 🐛 Troubleshooting

### vcpkg not found
Ensure `VCPKG_ROOT` environment variable is set and vcpkg is bootstrapped.

### Qt5 not found on Linux
Install system dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev libxcb-xinerama0

# Fedora
sudo dnf install mesa-libGL-devel
```

### Build fails
1. Clear the build directory: `rm -rf build`
2. Ensure vcpkg is up to date: `cd $VCPKG_ROOT && git pull`
3. Rebuild from scratch
