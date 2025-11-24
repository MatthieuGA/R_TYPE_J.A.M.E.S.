# 📘 R-Type — Networked Multiplayer Game (J.A.M.E.S.)

*A modern C++23 multiplayer shoot’em up with an original custom engine, an authoritative server, and a Qt-based client.*

---

## 📌 Overview

**R-Type J.A.M.E.S.** is a complete recreation of the classic 1987 arcade R-Type using modern C++ engineering principles.
The project features:

* A fully custom **Entity–Component–System (ECS)** engine
* A **multithreaded authoritative server**
* A **Qt graphical client**
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
* Qt rendering (players, enemies, missiles, starfield)
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

## 🧩 Architecture Overview

```
┌───────────────────────────────────────────────────────────┐
│                           ENGINE                          │
│  ECS (entities, components, systems)                      │
│  Rendering abstraction                                    │
│  Networking abstraction                                   │
│  Input abstraction                                        │
│  Resources & events                                       │
└───────────────────────────────────────────────────────────┘

                ▲                               ▲

┌───────────────┴──────────────┐   ┌────────────┴─────────────────┐
│             SERVER           │   │            CLIENT            │
│ - Authoritative logic        │   │ - Qt rendering               │
│ - Game loop (fixed step)     │   │ - Input management           │
│ - Snapshot broadcasting      │   │ - Snapshot interpolation     │
│ - UDP packet processing      │   │ - Audio & UI                 │
└──────────────────────────────┘   └──────────────────────────────┘
```

---

## 🔌 Networking

### **Protocol**

* Binary-only protocol
* Built on UDP
* Sequence numbers + timestamps
* Input packets (client → server)
* Snapshot packets (server → client)
* Entity create / update / destroy events

### **Client-side networking**

* Snapshot reception
* Reorder buffer
* Timeline interpolation
* Rendering at 60 FPS

### **Server-side networking**

* Network thread (receiver)
* Game thread (authoritative logic)
* Broadcaster thread (snapshots)
* Clean disconnect handling

---

## ⚙️ Build Instructions

### **Requirements**

* Linux (mandatory)
* Windows optional
* CMake ≥ 3.20
* g++ / clang++ supporting C++23
* vcpkg or Conan for dependencies
* Qt 6

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

```
/engine/             → Custom ECS & engine core
/server/             → Authoritative server
/client/             → Qt client (graphics, audio, input)
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

Documentation includes:

* Architecture documentation
* ECS specification
* Protocol RFC
* Engine overview
* Advanced feature documentation (Part 2)
* Implementation details
* Comparative study
* Accessibility documentation

All docs are available under `/docs`.

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

## 👥 Team J.A.M.E.S.

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
