# 📘 R-Type — Networked Multiplayer Game (J.A.M.E.S.)

*A modern C++23 multiplayer shoot’em up with an original custom engine, an authoritative server, and a SFML-based client.*

---

## 📌 Overview

**R-Type J.A.M.E.S.** is a complete recreation of the classic 1987 arcade R-Type using modern C++ engineering principles.
The project features:

* A fully custom **Entity–Component–System (ECS)** engine
* A **multithreaded authoritative server**
* A **SFML graphical client**
* A fully documented **binary UDP protocol**
* Smooth rendering, interpolation, and multiplayer support
* Professional-grade workflow (CI, tests, docs, RFCs, milestones)

This repository contains both the engine and the game powered by it.

---

## 🎮 Features

### ✅ Part 1 — Prototype (First Delivery)

* Custom ECS core
* Server-side authoritative gameplay
* Multithreaded server loop
* UDP networking (inputs → server, snapshots → clients)
* 4-player support
* SFML rendering (players, enemies, missiles, starfield)
* Client interpolation
* Collisions (AABB)
* Basic enemy behaviour
* Preliminary binary UDP protocol
* Crash-proof networking

---

### 🚀 Part 2 — Advanced Features (Second Delivery)

#### **Advanced Gameplay**

* Boss logic
* Advanced enemy behaviours
* Power-ups, charge shots
* Level design tools
* Custom asset pipelines
* Accessibility features

---

## 📋 Prerequisites

* **C++ Compiler** (GCC 12+ / MSVC 2022+ / Clang 15+)
* **CMake** 3.23 or higher
* **Package Manager** (one of the following):
  * **vcpkg** (recommended) - configured with `VCPKG_ROOT` environment variable
  * **Conan** 2.x - installed via pip (`pip install conan`)

## 🚀 Build & Run

### Using Build Scripts (Recommended)

The build scripts automatically detect and use vcpkg or Conan:

**Linux:**

```bash
./scripts/build.sh
```

**Windows:**

```cmd
scripts\build.bat
```

### Force a Specific Package Manager

You can force a specific package manager using environment variables:

**Linux:**

```bash
# Force vcpkg
FORCE_PACKAGE_MANAGER=vcpkg ./scripts/build.sh

# Force Conan
FORCE_PACKAGE_MANAGER=conan ./scripts/build.sh
```

**Windows:**

```cmd
REM Force vcpkg
set FORCE_PACKAGE_MANAGER=vcpkg
scripts\build.bat

REM Force Conan
set FORCE_PACKAGE_MANAGER=conan
scripts\build.bat
```

### Manual Build with vcpkg

```bash
rm -rf build && mkdir build && cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release && cd ..
```

### Manual Build with Conan

```bash
# Install dependencies
conan install . --output-folder=build --build=missing -s build_type=Release

# Configure and build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Then run:

```bash
./build/client/r-type_client
```

## 📦 Dependencies

Dependencies are automatically installed via vcpkg or Conan during CMake configuration:

* **SFML 2.6.x** - Graphics, window, network, audio, system
* **Boost.Asio** - Asynchronous I/O and networking
* **Boost.System** - System error codes for Boost libraries
* **Boost.Lockfree** - Lock-free data structures for high-performance concurrent operations
* **nlohmann_json** - JSON parsing library
* **fmt** - Modern formatting library
* **GoogleTest** - Unit testing framework (fetched automatically via CMake)

---

## 🔧 Alternative: Using Build Scripts

**Linux:**

```bash
./scripts/build.sh
```

**Windows:**

```cmd
scripts\build.bat
```

---

### **Clone the repository**

```bash
git clone https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S.
cd R_TYPE_J.A.M.E.S.
```

### **Configure**

```bash
cmake -S . -B build
```

### **Build**

```bash
cmake --build build -j
```

### **Run**

**Server:**

```bash
./build/bin/r-type_server <port>
```

**Client:**

```bash
./build/bin/r-type_client <server-ip> <port>
```

---

## 📁 Repository Structure

```txt
/engine/             → Custom ECS & engine core
/server/             → Authoritative server
/client/             → SFML client (graphics, audio, input)
/docs/               → Documentation, architecture, RFCs
/tests/              → Unit and functional tests
/assets/             → Sprites, sounds, UI
```

---

## 🧪 Testing

### **Unit Tests**

* Engine tests
* Server logic tests
* Component & system tests

### **Functional Tests**

* Client ↔ Server connectivity
* Snapshot correctness
* Multi-player sync tests

---

## 🛠️ Engineering Practices

* C++23
* CMake
* Dependency manager: vcpkg / conan
* GitHub Actions CI (build + format + tests)
* clang-format (Google style)
* Git hooks (pre-commit & commit-msg)
* RFC workflow for all major features
* Documentation auto-generated with Doxygen + Docusaurus

---

## 📚 Documentation

Comprehensive documentation is available at: [https://matthieuga.github.io/R_TYPE_J.A.M.E.S./](https://matthieuga.github.io/R_TYPE_J.A.M.E.S./)

Documentation includes:

* [Technology Choices and Comparative Study](docs/TECHNOLOGY_CHOICES.md) — Why SFML, Boost.Asio, and vcpkg
* Architecture documentation
* [ECS specification](docs/rfcs/RFC-0001-engine-architecture.md) — Entity-Component-System design
* Protocol RFC
* Engine overview
* Advanced feature documentation (Part 2)
* Implementation details
* Accessibility documentation

### Accessing local doc

All docs are available under `/docs`.

### Running Documentation Locally

The project uses [Docusaurus](https://docusaurus.io/) for documentation.

#### Prerequisites

* Node.js 20.0 or higher
* npm (comes with Node.js)

#### Setup and Run

1. Navigate to the docs directory:

    ```bash
    cd docs
    ```

2. Install dependencies (first time only):

    ```bash
    npm install
    ```

3. Start the development server:

    ```bash
    npm start
    ```

The documentation will be available at [http://localhost:3000](http://localhost:3000)

#### Build Documentation

To build the documentation for production:

```bash
cd docs
npm run build
```

The static files will be generated in the `docs/build` directory.

#### Serve Built Documentation

To serve the production build locally:

```bash
cd docs
npm run serve
```

---

## 📦 Releases

### **v0.5.0 — MVP**

* Fully playable R-Type prototype
* 4-player multiplayer
* Stable snapshots
* ECS complete
* Packaging scripts
* Partial documentation

### **v1.0.0 — Final Release**

* Advanced features from Part 2
* Final documentation site
* Accessibility compliance
* Engine as reusable module
* Full presentation-ready release

---

## 👥 Team J.A.M.E.S

| Member      | Role                     |
| ----------- | ------------------------ |
| J.          | Jocelyn                  |
| A.          | Arthuryan                |
| M.          | Matthieu                 |
| E.          | Enoal                    |
| S.          | Samuel                   |

---

## 🤝 Contributing

* Use feature branches
* PR required for all changes
* 1 reviewer minimum
* Gitmoji + English commit messages
* No direct commits to `main`
* CI must pass

See `CONTRIBUTING.md` for details.

## ℹ️ Additional Information

* **Package Manager Setup**:
  * **vcpkg**: See [vcpkg.io](https://vcpkg.io/) for installation
  * **Conan**: Install via `pip install conan`, then run `conan profile detect`
* **Cross-platform**: Works on Linux and Windows
* **Authors**: J.A.M.E.S. Development Team
