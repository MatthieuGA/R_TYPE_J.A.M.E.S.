# R_TYPE_J.A.M.E.S.

A multiplayer networked game engine implementation of the classic R-Type game.

## 📋 Prerequisites

- **C++ Compiler** (GCC 7+ / MSVC 2019+ / Clang 11+)
- **CMake** 3.21 or higher
- **vcpkg** (configured with `VCPKG_ROOT` environment variable)

## 🚀 Build & Run

```bash
rm -rf build && mkdir build && cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release && cd ..
```

Then run:
```bash
./r-type_client
```

## 📦 Dependencies

Dependencies (SFML, Asio) are automatically installed via vcpkg during CMake configuration

---

## 🔧 Alternative: Using Build Scripts

**Linux:**
```bash
./build.sh
```

**Windows:**
```cmd
build.bat
```

---

## ℹ️ Additional Information

- **vcpkg setup**: If you don't have vcpkg, see [vcpkg.io](https://vcpkg.io/)
- **Cross-platform**: Works on Linux and Windows
- **Authors**: J.A.M.E.S. Development Team
