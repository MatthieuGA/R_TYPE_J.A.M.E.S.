------

sidebar_position: 1sidebar_position: 1

------



# Getting Started# Tutorial Intro



Welcome to the **R-TYPE J.A.M.E.S.** documentation!Let's discover **Docusaurus in less than 5 minutes**.



R-TYPE J.A.M.E.S. is a modern C++ implementation of the classic R-TYPE game, featuring:## Getting Started



- **Client-Server Architecture**: Networked multiplayer gameplayGet started by **creating a new site**.

- **Custom Game Engine**: Built with modern C++ practices

- **ECS (Entity Component System)**: Efficient and scalable game architectureOr **try Docusaurus immediately** with **[docusaurus.new](https://docusaurus.new)**.

- **Cross-platform**: Support for multiple platforms

### What you'll need

## Prerequisites

- [Node.js](https://nodejs.org/en/download/) version 20.0 or above:

Before you begin, ensure you have the following installed:  - When installing Node.js, you are recommended to check all checkboxes related to dependencies.



- **CMake** (version 3.20 or higher)## Generate a new site

- **C++ Compiler** with C++17 support (GCC, Clang, or MSVC)

- **Git**Generate a new Docusaurus site using the **classic template**.



## Quick StartThe classic template will automatically be added to your project after you run the command:



### Building the Project```bash

npm init docusaurus@latest my-website classic

1. Clone the repository:```

```bash

git clone https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S.gitYou can type this command into Command Prompt, Powershell, Terminal, or any other integrated terminal of your code editor.

cd R_TYPE_J.A.M.E.S.

```The command also installs all necessary dependencies you need to run Docusaurus.



2. Configure the project:## Start your site

```bash

cmake -S . -B buildRun the development server:

```

```bash

3. Build the project:cd my-website

```bashnpm run start

cmake --build build -j$(nproc)```

```

The `cd` command changes the directory you're working with. In order to work with your newly created Docusaurus site, you'll need to navigate the terminal there.

### Running the Game

The `npm run start` command builds your website locally and serves it through a development server, ready for you to view at http://localhost:3000/.

#### Start the Server

```bashOpen `docs/intro.md` (this page) and edit some lines: the site **reloads automatically** and displays your changes.

./build/server/r-type_server
```

#### Start the Client
```bash
./build/client/r-type_client
```

## Running Tests

Run the test suite:
```bash
cd build && ctest --output-on-failure
```

## Next Steps

- Learn about the [Architecture](./architecture.md) of the game
- Understand the [Network Protocol](./protocol.md)
- Explore the API documentation
