# Build type: debug, sanitize, or release
BUILD_TYPE ?= debug

# Compiler and flags
CC = gcc
#CFLAGS = -Wall -O0 -g3 -ggdb3 -fno-omit-frame-pointer -fno-inline


#Set flags by build type
ifeq ($(BUILD_TYPE),debug)
  CFLAGS   = -DDEBUG -g -O0 -fno-omit-frame-pointer -fno-aggressive-loop-optimizations
  LDFLAGS  =
else ifeq ($(BUILD_TYPE),sanitize)
  CFLAGS   = -DDEBUG -g -O0 -fno-omit-frame-pointer -fno-aggressive-loop-optimizations \
             -fsanitize=address
  LDFLAGS  = -fsanitize=address
else ifeq ($(BUILD_TYPE),release)
  CFLAGS   = -O3 -march=native -flto -funroll-loops
  LDFLAGS  = -flto
else
  $(error Unknown BUILD_TYPE '$(BUILD_TYPE)')
endif


# Build directory structure (using absolute paths)
BUILD_DIR := $(abspath build)
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := $(BUILD_DIR)/lib

# List of all modules
MODULES := ndlib cpu machine devices debugger frontend/nd100x

# Export variables for module makefiles
export BUILD_TYPE
export CC CFLAGS LDFLAGS
export BUILD_DIR OBJ_DIR LIB_DIR
export MKPTYPES := $(TOOLS_DIR)/mkptypes

# Default target
.DEFAULT_GOAL := all

# Create build directories
create_dirs:
	@mkdir -p $(LIB_DIR)
	@$(foreach dir,$(MODULES),mkdir -p $(OBJ_DIR)/$(dir);)

# Build mkptypes tool using its own Makefile
$(TOOLS_DIR)/mkptypes:
	@echo "Building mkptypes tool..."
	@$(MAKE) -C tools/mkptypes

# Main targets
all: | create_dirs $(TOOLS_DIR)/mkptypes
	@$(foreach module,$(MODULES),\
		echo "Building module: $(module)" && \
		if [ -f src/$(module)/Makefile ]; then \
			$(MAKE) -C src/$(module) all || exit 1; \
		else \
			echo "Warning: No Makefile in src/$(module)"; \
		fi;)

# Clean everything
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all debug sanitize release clean create_dirs 

debug:   BUILD_TYPE = debug
debug:   all

sanitize: BUILD_TYPE = sanitize
sanitize: all

release: BUILD_TYPE = release
release: all

# Pattern rule for object files
$(MODULE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

$(MODULES): create_dirs $(TOOLS_DIR)/mkptypes
	@echo "Building module: $@"
	@$(MAKE) -C src/$@ all