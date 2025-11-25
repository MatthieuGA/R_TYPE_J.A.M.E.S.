# R-TYPE J.A.M.E.S.# R-TYPE J.A.M.E.S.# R_TYPE_J.A.M.E.S.



A networked multiplayer game engine inspired by the classic R-Type game.



## PrerequisitesA networked multiplayer game engine inspired by the classic R-Type game.A multiplayer networked game engine implementation of the classic R-Type game, built with modern C++ and featuring a custom game engine architecture.



### 1. CMake (version 3.17 or higher) - REQUIRED



**Windows:**## Prerequisites## 🎮 About

- **Recommended**: Install via winget (run in PowerShell):

  ```powershell

  winget install Kitware.CMake

  ```To build this project, you need:This project implements a multi-threaded server and graphical client for R-Type using a custom-designed game engine. The architecture demonstrates advanced C++ development techniques and follows software engineering best practices.

- Or download installer from https://cmake.org/download/ (check "Add CMake to system PATH" during install)

- Or via Chocolatey: `choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'`



**Linux (Ubuntu/Debian):**- **CMake** (version 3.17 or higher)## 📋 Requirements

```bash

sudo apt update && sudo apt install cmake build-essential- **C++20 compiler**

```

  - Windows: MSVC (Visual Studio 2019+) or MinGW-w64- **C++ Compiler**: GCC 7+ (Linux) or MSVC 2019+ (Windows)

**Verify installation (restart terminal first):**

```bash  - Linux: GCC 10+ or Clang 11+- **CMake**: 3.21 or higher

cmake --version

```- **Git** (to clone vcpkg)- **vcpkg**: For dependency management



### 2. C++20 Compiler- **Git**: For version control



- **Windows**: MSVC (Visual Studio 2019+ with C++ workload) or MinGW-w64**That's it!** vcpkg is included as a submodule and will be automatically bootstrapped during the build process.

- **Linux**: GCC 10+ or Clang 11+

## 🛠️ Dependencies

### 3. Git

## Building the Project

For cloning vcpkg (already included as submodule)

The project uses vcpkg for cross-platform dependency management:

---

### Quick Start

**That's it!** vcpkg is included as a submodule and will be automatically bootstrapped during the build process.

- **Qt5**: GUI framework (Core, Widgets)

**⚠️ Important:** After installing CMake, **restart your terminal or IDE** for PATH changes to take effect.

**Windows:**- **Asio**: Asynchronous networking library

## Building the Project

```cmd

### Quick Start

build.batAll dependencies are automatically managed through vcpkg manifest mode.

**Windows (PowerShell or CMD):**

```cmd```

build.bat

```## 🚀 Quick Start



**Linux or WSL:****Linux:**

```bash

chmod +x build.sh```bash### 1. Install vcpkg

./build.sh

```chmod +x build.sh



**Git Bash on Windows:**./build.sh**Linux/macOS:**

```bash

chmod +x build_unified.sh``````bash

./build_unified.sh

```git clone https://github.com/microsoft/vcpkg.git



### Manual Build Steps### Manual Build Stepscd vcpkg



If you prefer to build manually:./bootstrap-vcpkg.sh



**Windows:**If you prefer to build manually:export VCPKG_ROOT=$(pwd)

```cmd

REM Bootstrap vcpkg (first time only)export PATH=$VCPKG_ROOT:$PATH

cd vcpkg

bootstrap-vcpkg.bat**Windows:**```

cd ..

```cmd

REM Build project

mkdir buildREM Bootstrap vcpkg (first time only)**Windows (PowerShell):**

cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Releasecd vcpkg```powershell

cmake --build . --config Release

cd ..bootstrap-vcpkg.batgit clone https://github.com/microsoft/vcpkg.git

```

cd ..cd vcpkg

**Linux:**

```bash.\bootstrap-vcpkg.bat

# Bootstrap vcpkg (first time only)

cd vcpkgREM Build project$env:VCPKG_ROOT = $PWD

./bootstrap-vcpkg.sh

cd ..mkdir build$env:PATH += ";$env:VCPKG_ROOT"



# Build projectcd build```

mkdir -p build

cd buildcmake .. -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Releasecmake --build . --config Release### 2. Clone and Build

cd ..

```cd ..



## Running the Project```**Linux:**



After building:```bash



**Windows:****Linux:**git clone <repository-url>

```cmd

r-type_client.exe```bashcd R_TYPE_J.A.M.E.S.

```

# Bootstrap vcpkg (first time only)mkdir build && cd build

**Linux:**

```bashcd vcpkgcmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

./r-type_client

```./bootstrap-vcpkg.shcmake --build . --config Release



## Project Structurecd ..```



```

R_TYPE_JAMES/

├── src/              # Source code# Build project**Windows:**

│   ├── main.cpp      # Entry point

│   └── CMakeLists.txtmkdir -p build```powershell

├── vcpkg/            # Package manager (submodule)

├── build/            # Build output (generated)cd buildgit clone <repository-url>

├── CMakeLists.txt    # Main CMake configuration

├── vcpkg.json        # Package dependenciescmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Releasecd R_TYPE_J.A.M.E.S.

├── build.bat         # Windows build script

├── build.sh          # Linux build scriptcmake --build . --config Releasemkdir build

├── build_unified.sh  # Cross-platform build script (for Git Bash on Windows)

└── README.md         # This filecd ..cd build

```

```cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

## Dependencies

cmake --build . --config Release

This project uses the following libraries (automatically installed via vcpkg):

## Running the Project```

- **SFML** - Simple and Fast Multimedia Library (graphics, window, network, audio)

- **Asio** - Asynchronous I/O library for networking



## TroubleshootingAfter building:### 3. Run



### "CMake is not installed or not in PATH"



Install CMake using one of the methods in the Prerequisites section, then **restart your terminal**.**Windows:****Linux:**



### vcpkg is not found```cmd```bash



Make sure you've cloned the repository with submodules:r-type_client.exe./r-type_client

```bash

git submodule update --init --recursive``````

```



Or clone vcpkg manually:

```bash**Linux:****Windows:**

git clone https://github.com/microsoft/vcpkg.git

``````bash```powershell



### Build fails on Linux with "cannot find -lGL"./r-type_client.\Release\r-type_client.exe



Install OpenGL development libraries:``````

```bash

# Ubuntu/Debian

sudo apt-get install libgl1-mesa-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev libx11-dev

## Project Structure### Adding Dependencies

# Fedora

sudo dnf install mesa-libGL-devel libXrandr-devel libXcursor-devel libXi-devel libXinerama-devel libX11-devel

```

```To add new dependencies, edit `vcpkg.json` and rebuild:

### Build fails on Windows with "LINK : fatal error"

R_TYPE_JAMES/```json

Make sure you have Visual Studio Build Tools or Visual Studio installed with C++ development workload.

├── src/              # Source code{

### Running from Git Bash on Windows

│   ├── main.cpp      # Entry point  "dependencies": [

If you're using Git Bash (MINGW64) on Windows, use `build_unified.sh` which will automatically call the Windows batch script:

```bash│   └── CMakeLists.txt    "new-package-name"

./build_unified.sh

```├── vcpkg/            # Package manager (submodule)  ]



Or call the batch file directly via cmd:├── build/            # Build output (generated)}

```bash

cmd.exe /c build.bat├── CMakeLists.txt    # Main CMake configuration```

```

├── vcpkg.json        # Package dependencies

## Development

├── build.bat         # Windows build script## 🌐 Cross-Platform Support

### Adding New Dependencies

├── build.sh          # Linux build script

To add a new dependency, simply edit `vcpkg.json` and add the package name to the `dependencies` array:

└── README.md         # This fileThis project is designed to run on:

```json

{```- ✅ Linux (Ubuntu 20.04+, Fedora, Arch)

  "name": "r-type",

  "version": "1.0.0",- ✅ Windows (10/11 with MSVC)

  "dependencies": [

    "sfml",## Dependencies

    "asio",

    "your-new-package"

  ]

}This project uses the following libraries (automatically installed via vcpkg):## 👥 Authors

```



Then rebuild the project. vcpkg will automatically install the new dependency.

- **SFML** - Simple and Fast Multimedia Library (graphics, window, network, audio)- Development Team: J.A.M.E.S.

### Clean Build

- **Asio** - Asynchronous I/O library for networking

To perform a clean build:

## 🔗 Useful Links

**Windows:**

```cmd## Troubleshooting

rmdir /s /q build

build.bat- [Project Documentation](docs/)

```

### vcpkg is not found- [vcpkg Documentation](https://vcpkg.io/)

**Linux:**

```bash- [CMake Documentation](https://cmake.org/documentation/)

rm -rf build

./build.shMake sure you've cloned the repository with submodules:

```

```bash## 🐛 Troubleshooting

## License

git submodule update --init --recursive

[Your License Here]

```### vcpkg not found

## Authors

Ensure `VCPKG_ROOT` environment variable is set and vcpkg is bootstrapped.

J.A.M.E.S. Team

Or clone vcpkg manually:

```bash### Qt5 not found on Linux

git clone https://github.com/microsoft/vcpkg.gitInstall system dependencies:

``````bash

# Ubuntu/Debian

### Build fails on Linux with "cannot find -lGL"sudo apt-get install libgl1-mesa-dev libxcb-xinerama0



Install OpenGL development libraries:# Fedora

```bashsudo dnf install mesa-libGL-devel

# Ubuntu/Debian```

sudo apt-get install libgl1-mesa-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev libx11-dev

### Build fails

# Fedora1. Clear the build directory: `rm -rf build`

sudo dnf install mesa-libGL-devel libXrandr-devel libXcursor-devel libXi-devel libXinerama-devel libX11-devel2. Ensure vcpkg is up to date: `cd $VCPKG_ROOT && git pull`

```3. Rebuild from scratch


### Build fails on Windows with "LINK : fatal error"

Make sure you have Visual Studio Build Tools or Visual Studio installed with C++ development workload.

## Development

### Adding New Dependencies

To add a new dependency, simply edit `vcpkg.json` and add the package name to the `dependencies` array:

```json
{
  "name": "r-type",
  "version": "1.0.0",
  "dependencies": [
    "sfml",
    "asio",
    "your-new-package"
  ]
}
```

Then rebuild the project. vcpkg will automatically install the new dependency.

### Clean Build

To perform a clean build:

**Windows:**
```cmd
rmdir /s /q build
build.bat
```

**Linux:**
```bash
rm -rf build
./build.sh
```

## License

[Your License Here]

## Authors

J.A.M.E.S. Team
