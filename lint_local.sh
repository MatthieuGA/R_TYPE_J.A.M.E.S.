#!/bin/bash
# Local cpplint script - matches CI behavior
# Only lints project source files, excludes dependencies and generated files

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Running cpplint on project files...${NC}"

# Find only project source files, excluding:
# - tests/ (if you want to exclude tests)
# - build/
# - vcpkg/
# - _deps/
# - Any vcpkg_installed/
# - bin/, lib/
find . -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' -o -name '*.cc' \) \
    ! -path './build/*' \
    ! -path './vcpkg/*' \
    ! -path './_deps/*' \
    ! -path './vcpkg_installed/*' \
    ! -path './bin/*' \
    ! -path './lib/*' \
    ! -path '*/vcpkg_installed/*' \
    ! -path '*/CMakeFiles/*' \
    | xargs cpplint \
        --repository=. \
        --quiet \
        --output=vs7 \
        --filter=-legal/copyright,-build/c++17,+build/c++23,-runtime/references,-whitespace/indent_namespace

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ No cpplint issues found!${NC}"
    exit 0
else
    echo -e "${RED}✗ Cpplint found style issues${NC}"
    exit 1
fi
