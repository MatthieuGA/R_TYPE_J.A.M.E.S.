---
id: getting-started
title: Getting Started
sidebar_position: 1
---

# Getting Started

Welcome to the official documentation of **R-TYPE J.A.M.E.S.**  
This guide explains **the purpose of the project**, **how it works**, and **how to install and run it** on your machine.

---

## 🎯 Project Goal

**R-TYPE J.A.M.E.S.** is a modern C++ recreation of the iconic arcade game **R-Type**, redesigned to work as a **networked multiplayer game**.

The objective of the project is twofold:

### 1. For Players  
Provide a simple and enjoyable way to play a **multiplayer R-Type experience**, where several players can connect together, fight enemies, and progress through the game.

### 2. For Developers  
Offer a **clean, modern, and modular codebase** that showcases:

- A fully custom **client–server architecture**
- A lightweight **game engine** developed from scratch
- An **Entity Component System (ECS)** for scalable game logic
- Modern C++ development practices (CMake, package managers, documentation)
- A clear, documented **binary UDP protocol** for real-time gameplay

This documentation helps both players and developers understand **how to use**, **compile**, and **extend** the project.

---

## 🚀 What You Can Do With R-TYPE J.A.M.E.S.

With this project, you can:

- Launch a dedicated **game server**  
- Connect one or more **clients** to play together
- Control your spaceship, fight enemies, shoot missiles, and survive waves of Bydos
- Observe real-time synchronized multiplayer gameplay
- Modify or extend the engine if you are a developer

This documentation will guide you through all these steps.

---

## 📦 Prerequisites

To build and run the project, you will need:

- **CMake** version 3.20 or higher  
- A **C++17-compatible compiler** (GCC, Clang, or MSVC)  
- **Git**  
- A dependency **package manager** (Conan, vcpkg, or CMake CPM) depending on your setup

---

## ⚙️ Installation & Build

### 1. Clone the Repository

```bash
git clone https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S.git
cd R_TYPE_J.A.M.E.S.
```

2. Build the Project
```bash
./build.sh
or
./build.bat
```

🎮 Running the Game
Start the Server

The server hosts the entire game logic.
Run it first:

```bash
./build/server/r-type_server
```

Start the Client

Each client represents one player and displays the game:
```bash
./build/client/r-type_client
```

You can launch multiple clients on the same machine or across the network.

🧪 Running the Test Suite

To execute unit and integration tests:

```bash
cd build && ctest --output-on-failure
```

📚 Next Steps
- To continue exploring the project:
- Learn how the game engine is structured
- Understand the network protocol between server and clients
- Read tutorials on how to extend or modify the engine
- Explore the API documentation for developers