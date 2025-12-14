#!/bin/bash
# Lint script for R-TYPE J.A.M.E.S. - runs cpplint on project code only
# Excludes: tests, build directory, and vcpkg dependencies

set -e

cd "$(dirname "$0")"

echo "Running cpplint on project code (excluding tests, build, and dependencies)..."
echo ""

find ./engine ./server ./client -type f \( -name '*.cpp' -o -name '*.hpp' \) \
  ! -path '*/CMakeFiles/*' \
  ! -path '*/.git/*' \
  | xargs cpplint \
    --repository=. \
    --quiet \
    --output=vs7 \
    --filter=-legal/copyright,-build/c++17,+build/c++23,-runtime/references,-whitespace/indent_namespace

echo ""
echo "✓ Linting complete"
