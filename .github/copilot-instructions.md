# GitHub Copilot Instructions

## Priority Guidelines

When generating code for this repository:

1. **Version Compatibility**: Strictly adhere to **C++23** standards and the specific library versions defined in `vcpkg.json` and `CMakeLists.txt`.
2. **Style Guide Compliance**: Follow the **Google C++ Style Guide** for naming and formatting, as explicitly defined in the Naming Conventions section below.
3. **Architectural Consistency**: Respect the Entity-Component-System (ECS) architecture and the Authoritative Server model.
4. **Code Quality**: Prioritize **Maintainability**, **Testability**, **Performance**, and **Security** in all generated code.
5. **Documentation**: Apply Doxygen-style documentation to all functions and classes.

## Technology Version Detection

Before generating code, ensure compatibility with:

1. **Language**: **C++23**

   - Use modern features (concepts, ranges, smart pointers) where appropriate.
   - Avoid raw pointers unless strictly necessary (e.g., interacting with legacy C APIs).

2. **Build System**: **CMake 3.23+**

   - Use `FetchContent` or `find_package` (via vcpkg) for dependencies.

3. **Key Libraries**:
   - **SFML**: 2.6.1+ (Graphics, Windowing, Audio)
   - **Asio**: Latest via vcpkg (Networking)
   - **GoogleTest**: 1.14.0 (Testing)

## Codebase Patterns

### Naming Conventions (Google C++ Style)

Strictly follow these naming rules, even if some existing files differ (prioritize these rules for new/refactored code):

| Element                 | Style            | Example                             | Note                                      |
| :---------------------- | :--------------- | :---------------------------------- | :---------------------------------------- |
| **Functions / Methods** | `UpperCamelCase` | `CalculateTotal()`, `ConnectToDb()` | Always start with a capital letter.       |
| **Variables (Local)**   | `snake_case`     | `user_index`, `file_path`           | Lowercase with underscores.               |
| **Parameters**          | `snake_case`     | `input_string`, `max_count`         | Same as local variables.                  |
| **Member Variables**    | `snake_case_`    | `table_name_`, `is_connected_`      | **Must** end with a trailing underscore.  |
| **Struct Members**      | `snake_case`     | `x`, `y`, `width`                   | No trailing underscore for POD structs.   |
| **Constants**           | `kCamelCase`     | `kMaxIterations`, `kPi`             | Starts with `k`, then CamelCase.          |
| **Accessors**           | `snake_case()`   | `processed_count()`                 | Matches variable name without underscore. |

### Documentation Style

- Use **Doxygen** style blocks `/** ... */` before functions and classes.
- Always include `@brief` for a concise description.
- Document all parameters using `@param`.
- Document return values using `@return` (if applicable).

**Example:**

```cpp
/**
 * @brief Adds a component to the specified entity.
 *
 * @param entity The entity to modify.
 * @param component The component data to add.
 * @return A reference to the added component.
 */
template <typename Component>
Component& AddComponent(Entity entity, Component&& component);
```

## Code Quality Standards

### Maintainability

- **Self-Documenting Code**: Use clear, descriptive names following the Google Style.
- **Single Responsibility**: Keep functions focused; split complex logic into helper methods.
- **Modern C++**: Prefer `std::unique_ptr` / `std::shared_ptr` over `new`/`delete`.

### Performance

- **Move Semantics**: Use `std::move` and r-value references to avoid unnecessary copies.
- **Pass-by-Reference**: Pass complex types by `const&` to avoid copying.
- **Container Optimization**: Use `reserve()` on vectors when size is known.
- **Const Correctness**: Mark methods and variables `const` wherever possible.

### Security

- **Input Validation**: Validate all data received from the network before processing.
- **Buffer Safety**: Avoid C-style arrays and `strcpy`; use `std::string` and `std::vector`.
- **Exception Safety**: Ensure code provides basic exception safety guarantees.

### Testability

- **Dependency Injection**: Design classes to accept dependencies (e.g., via constructor) to facilitate mocking.
- **Testable Units**: Avoid global state and singletons where possible.

## Testing Approach

### Unit Testing (GoogleTest)

- **Requirement**: Every new feature or logic change **must** include corresponding unit tests.
- **Location**: Place tests in the `tests/` directory, mirroring the source structure.
- **Naming**:
  - Test Files: `test_<component>.cpp` (e.g., `test_registry.cpp`)
  - Test Cases: `TEST(TestSuiteName, TestName)` (e.g., `TEST(RegistryTest, SpawnEntity)`)
- **Assertions**: Use `EXPECT_*` for non-fatal checks and `ASSERT_*` for fatal checks.

## Project-Specific Guidance

- **ECS Architecture**:
  - Components must be **POD** (Plain Old Data) structs.
  - Systems must contain **logic only** and iterate over components.
  - Entities are IDs (size_t).
- **Networking**:
  - Use **UDP** for gameplay data (position, events).
  - Use **TCP** only for critical control messages (if justified).
  - Ensure the server is **authoritative** (client predicts, server corrects).
- **Refactoring**: When editing existing files that do not follow the Google Style (e.g., `snake_case` methods), prefer updating them to `UpperCamelCase` if the scope allows, or follow the new standard for added code.
