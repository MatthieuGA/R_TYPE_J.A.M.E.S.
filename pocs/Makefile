# Graphics Library PoCs Makefile
# Usage:
#   make           - Build all PoCs
#   make sfml      - Build and run SFML PoC
#   make sdl2      - Build and run SDL2 PoC (when ready)
#   make raylib    - Build and run Raylib PoC (when ready)
#   make clean     - Clean build artifacts

BUILD_DIR := build
VCPKG_TOOLCHAIN := $(shell if [ -f "../vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then echo "../vcpkg/scripts/buildsystems/vcpkg.cmake"; elif [ -n "$$VCPKG_ROOT" ]; then echo "$$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"; else echo ""; fi)

.PHONY: all sfml sdl2 raylib clean help reconfigure

all: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building all PoCs..."
	cmake --build $(BUILD_DIR) -j

$(BUILD_DIR)/CMakeCache.txt:
	@echo "Configuring PoCs..."
	@if [ -z "$(VCPKG_TOOLCHAIN)" ]; then \
		echo "Warning: vcpkg toolchain not found. Building without vcpkg."; \
		cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release; \
	else \
		echo "Using vcpkg toolchain: $(VCPKG_TOOLCHAIN)"; \
		cmake -B $(BUILD_DIR) -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) -DCMAKE_BUILD_TYPE=Release; \
	fi

reconfigure:
	@echo "Reconfiguring PoCs..."
	@rm -rf $(BUILD_DIR)
	@$(MAKE) $(BUILD_DIR)/CMakeCache.txt

sfml: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building SFML PoC..."
	cmake --build $(BUILD_DIR) --target sfml_poc -j
	@echo "Running SFML PoC..."
	@cd $(BUILD_DIR)/sfml-demo && ./sfml_poc

sdl2: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building SDL2 PoC..."
	cmake --build $(BUILD_DIR) --target r-type_sdl2 -j
	@echo "Running SDL2 PoC..."
	@cd $(BUILD_DIR)/sdl2-demo && ./r-type_sdl2

raylib: $(BUILD_DIR)/CMakeCache.txt
	@echo "Building Raylib PoC..."
	cmake --build $(BUILD_DIR) --target raylib_poc -j
	@echo "Running Raylib PoC..."
	@cd $(BUILD_DIR)/raylib-demo && ./raylib_poc

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

help:
	@echo "Graphics Library PoCs - Makefile targets:"
	@echo ""
	@echo "  make              - Build all PoCs"
	@echo "  make sfml         - Build and run SFML PoC"
	@echo "  make sdl2         - Build and run SDL2 PoC (when ready)"
	@echo "  make raylib       - Build and run Raylib PoC (when ready)"
	@echo "  make clean        - Clean build artifacts"
	@echo "  make reconfigure  - Force CMake reconfiguration"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Ensure SFML is installed via vcpkg or system package manager."
