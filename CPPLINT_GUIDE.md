# CPPLint Usage Guide

## Quick Commands

Run linting checks using VS Code tasks:
- **Ctrl+Shift+P** → "Lint: Check All Files"
- **Ctrl+Shift+P** → "Lint: Check Current File"
- **Ctrl+Shift+P** → "Lint: Check Engine"
- **Ctrl+Shift+P** → "Lint: Check Tests"

Or from terminal:
```bash
# Check everything
cpplint --recursive engine/include engine/src tests/

# Check specific file
cpplint engine/include/zipper.hpp

# Check only engine
cpplint --recursive engine/
```

## Common Issues and Fixes

### 1. Whitespace/Indent Issues
**Problem**: `public:` should be indented +1 space inside class
**Fix**: Add 2 spaces before `public:`, `private:`, `protected:`

```cpp
// WRONG
class Foo {
public:
    void bar();
private:
    int x;
};

// RIGHT
class Foo {
 public:
    void bar();
 private:
    int x;
};
```

### 2. Line Length > 80 Characters
**Problem**: Lines should be <= 80 characters long
**Fix**: Break long lines

```cpp
// WRONG
friend bool operator==(zipper_iterator const& a, zipper_iterator const& b) {

// RIGHT
friend bool operator==(zipper_iterator const& a,
                       zipper_iterator const& b) {
```

### 3. Closing Parenthesis Position
**Problem**: Closing ) should be moved to the previous line
**Fix**: Put `)` on same line as last parameter

```cpp
// WRONG
using value_type = decltype(std::tuple_cat(
    std::tuple<std::size_t>{},
    std::declval<typename base_iterator::value_type>()
));

// RIGHT
using value_type = decltype(std::tuple_cat(
    std::tuple<std::size_t>{},
    std::declval<typename base_iterator::value_type>()));
```

### 4. Single-Parameter Constructors
**Problem**: Single-parameter constructors should be marked explicit
**Fix**: Add `explicit` keyword

```cpp
// WRONG
indexed_zipper_iterator(base_iterator it);

// RIGHT
explicit indexed_zipper_iterator(base_iterator it);
```

### 5. Trailing Whitespace
**Problem**: Line ends in whitespace
**Fix**: Remove spaces at end of lines
(Most editors have "Trim End of Line" option)

## Configuration

The `CPPLINT.cfg` file disables these checks:
- `-build/include` - Allow custom include directory paths
- `-legal/copyright` - Skip copyright header checks  
- `-runtime/references` - Allow non-const references (needed for ECS)

## Exit Codes
- **0**: No issues found
- **1**: Style issues found (non-blocking)
- **2+**: Fatal errors

## Useful Tips

1. **Run before committing**:
   ```bash
   cpplint --recursive engine/ && echo "✓ All good!"
   ```

2. **Check specific pattern**:
   ```bash
   cpplint engine/include/*.hpp
   ```

3. **Verbose mode**:
   ```bash
   cpplint -v engine/include/zipper.hpp
   ```

4. **Disable specific filters temporarily**:
   ```bash
   cpplint --filter=-whitespace/line_length engine/
   ```
