
# Makefile

# Variables
BACKEND := wgpu
DEV_MODE := off
BUILD_TYPE := Release
SCRIPT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(SCRIPT_DIR)/build

# Targets
.PHONY: all configure build clean

all: configure build

configure:
	@if [ "$(BACKEND)" = "dawn" ]; then \
		cmake -B "$(BUILD_DIR)" -DWEBGPU_BACKEND=DAWN -DDEV_MODE=$(DEV_MODE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) "$(SCRIPT_DIR)"; \
	elif [ "$(BACKEND)" = "wgpu" ]; then \
		cmake -B "$(BUILD_DIR)" -DWEBGPU_BACKEND=WGPU -DDEV_MODE=$(DEV_MODE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) "$(SCRIPT_DIR)"; \
	else \
		echo "Unknown backend: $(BACKEND)"; \
		exit 1; \
	fi

build:
	cmake --build "$(BUILD_DIR)"
	@echo "Done."

clean:
	rm -rf "$(BUILD_DIR)"
