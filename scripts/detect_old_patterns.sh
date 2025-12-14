#!/bin/bash
# detect_old_patterns.sh
# Script to detect old SFML-direct patterns in codebase
# Usage: ./scripts/detect_old_patterns.sh [directory]
# Default: scans entire project except build/, vcpkg/, old_engine/

set -e

# Colors for output
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default directory
SCAN_DIR="${1:-.}"

# Exclude patterns
EXCLUDE_DIRS="build|vcpkg|old_engine|_deps|node_modules|.git|docs/examples"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Old Engine Pattern Detection Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${GREEN}Scanning directory: ${SCAN_DIR}${NC}"
echo -e "${GREEN}Excluded: ${EXCLUDE_DIRS}${NC}"
echo ""

# Counters
TOTAL_ISSUES=0

# Function to search and display results
search_pattern() {
    local pattern="$1"
    local description="$2"
    local severity="$3"  # CRITICAL, WARNING, INFO
    
    # Color based on severity
    local color="${YELLOW}"
    if [ "$severity" = "CRITICAL" ]; then
        color="${RED}"
    elif [ "$severity" = "INFO" ]; then
        color="${GREEN}"
    fi
    
    echo -e "${color}[${severity}] ${description}${NC}"
    
    # Search for pattern
    local results=$(grep -rn --color=always "$pattern" "$SCAN_DIR" \
        --include="*.cpp" \
        --include="*.hpp" \
        --include="*.h" \
        --exclude-dir={build,vcpkg,old_engine,_deps,node_modules,.git} \
        2>/dev/null || true)
    
    if [ -n "$results" ]; then
        echo "$results"
        local count=$(echo "$results" | wc -l)
        TOTAL_ISSUES=$((TOTAL_ISSUES + count))
        echo -e "${color}  → Found ${count} occurrence(s)${NC}"
    else
        echo -e "  ✓ No occurrences found"
    fi
    echo ""
}

echo -e "${BLUE}=== CRITICAL: Must Fix (Breaking Changes) ===${NC}"
echo ""

search_pattern "sf::RenderWindow" "Direct SFML RenderWindow usage" "CRITICAL"
search_pattern "window_\.draw\(" "Direct window_.draw() calls" "CRITICAL"
search_pattern "window_\.clear\(" "Direct window_.clear() calls" "CRITICAL"
search_pattern "window_\.display\(" "Direct window_.display() calls" "CRITICAL"
search_pattern "\.loadFromFile\(" "Manual resource loading with loadFromFile()" "CRITICAL"
search_pattern "sf::Sprite\s" "SFML Sprite object usage" "CRITICAL"
search_pattern "sf::Texture\s" "SFML Texture object usage" "CRITICAL"
search_pattern "sf::Font\s" "SFML Font object usage" "CRITICAL"
search_pattern "sf::Text\s" "SFML Text object usage" "CRITICAL"

echo ""
echo -e "${BLUE}=== WARNING: Should Migrate (Coupling Issues) ===${NC}"
echo ""

search_pattern "setPosition\(" "Manual sprite/text positioning" "WARNING"
search_pattern "setScale\(" "Manual sprite/text scaling" "WARNING"
search_pattern "setRotation\(" "Manual sprite/text rotation" "WARNING"
search_pattern "setTexture\(" "Manual texture setting" "WARNING"
search_pattern "setFont\(" "Manual font setting" "WARNING"
search_pattern "setColor\(" "Manual color setting" "WARNING"
search_pattern "setTextureRect\(" "Manual texture rect setting" "WARNING"
search_pattern "setOrigin\(" "Manual origin setting" "WARNING"
search_pattern "getLocalBounds\(\)" "Direct bounds access" "WARNING"
search_pattern "getGlobalBounds\(\)" "Direct bounds access" "WARNING"

echo ""
echo -e "${BLUE}=== INFO: Check for Updates (Type Changes) ===${NC}"
echo ""

search_pattern "sf::Color" "SFML Color type (should be Engine::Graphics::Color)" "INFO"
search_pattern "sf::Vector2f" "SFML Vector2f type (should be Engine::Graphics::Vector2f)" "INFO"
search_pattern "sf::IntRect" "SFML IntRect type (should be Engine::Graphics::IntRect)" "INFO"
search_pattern "sf::FloatRect" "SFML FloatRect type (should be Engine::Graphics::FloatRect)" "INFO"
search_pattern "sf::Event" "SFML Event type (should be Engine::Graphics::Event)" "INFO"
search_pattern "sf::Keyboard" "SFML Keyboard usage (use RenderingEngine::IsKeyPressed)" "INFO"
search_pattern "sf::Mouse" "SFML Mouse usage (use RenderingEngine::GetMousePosition)" "INFO"

echo ""
echo -e "${BLUE}=== INFO: GameWorld Access Patterns ===${NC}"
echo ""

search_pattern "game_world\.window_" "Direct GameWorld window_ access (should use rendering_engine_)" "CRITICAL"
search_pattern "video_backend_" "Old video_backend_ field (should be rendering_engine_)" "WARNING"
search_pattern "GetWindow\(\)" "GetWindow() method (no longer exists)" "CRITICAL"

echo ""
echo -e "${BLUE}=== INFO: Include Statements ===${NC}"
echo ""

search_pattern "#include <SFML/" "SFML includes (should use Engine abstractions)" "INFO"
search_pattern "PluginVideoBackend" "Old PluginVideoBackend usage (should be RenderingEngine)" "WARNING"

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}========================================${NC}"
if [ $TOTAL_ISSUES -eq 0 ]; then
    echo -e "${GREEN}✓ No old patterns detected! Code is clean.${NC}"
else
    echo -e "${YELLOW}⚠ Found ${TOTAL_ISSUES} potential issue(s)${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Review each occurrence above"
    echo "  2. Use API_TRANSLATION_TABLE.md for replacements"
    echo "  3. Run tests after changes"
    echo "  4. Re-run this script to verify fixes"
fi
echo ""
echo -e "${BLUE}For detailed migration instructions:${NC}"
echo "  - API Translation: docs/API_TRANSLATION_TABLE.md"
echo "  - Migration Guide: docs/RENDERING_ENGINE_MIGRATION.md"
echo "  - Breaking Changes: docs/BREAKING_CHANGES.md"
echo "  - Examples: docs/MIGRATION_EXAMPLES.md"
echo ""

exit 0
