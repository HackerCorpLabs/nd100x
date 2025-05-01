# Source directory (relative to this Makefile)
SRC_DIR := .

# Source files in dependency order
SRCS := config.c nd100x.c

# Frontend module specific flags
CFLAGS += -D_GNU_SOURCE -DDEBUG

# Module name
MODULE := nd100x

# Build paths - use parent's paths if available, otherwise set defaults
BUILD_DIR ?= ../../../build
OBJ_DIR ?= $(BUILD_DIR)/obj
LIB_DIR ?= $(BUILD_DIR)/lib
BIN_DIR ?= $(BUILD_DIR)/bin

# Build paths for this module
MODULE_OBJ_DIR := $(OBJ_DIR)/$(MODULE)
OBJS := $(SRCS:%.c=$(MODULE_OBJ_DIR)/%.o)
MODULE_BIN := $(BIN_DIR)/$(MODULE)

# Include paths
INCLUDES := -I$(SRC_DIR) -I../../cpu -I../../ndlib -I../../devices -I../../machine -I../../debugger

# Libraries to link against (order matters for resolution)
LIBS := -L$(LIB_DIR) \
	-Wl,--start-group \
	-lmachine \
	-ldevices \
	-lcpu \
	-lndlib \
	-lm \
	-Wl,--end-group

# Include LibDAP headers if available
ifdef LIBDAP_INCLUDE_DIR
    INCLUDES += -I$(LIBDAP_INCLUDE_DIR)
endif

# Link with LibDAP if available (and its dependencies)
ifdef LIBDAP_LIB_DIR
    LIBS += -L$(LIBDAP_LIB_DIR) -ldap -lcjson
endif

# point this at wherever you built mkptypes
MKPTYPES    := ./../../../tools/mkptypes/mkptypes

# Combine prototypes into nd100x_protos.h
nd100x_protos.h: $(SRCS)
	@echo "Creating combined prototype header $@"
	@echo "/* AUTO-GENERATED FILE. DO NOT EDIT! */" > $@
	@for src in $(SRCS); do \
		echo "/* Prototypes from $$src */" >> $@; \
		$(MKPTYPES) $$src >> $@; \
		echo "" >> $@; \
	done

# Create necessary directories
create_dirs:
	@mkdir -p $(MODULE_OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Build the executable
all: | create_dirs  $(MODULE_BIN)

# Link the executable
$(MODULE_BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

# Pattern rule for object files
$(MODULE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Header dependencies
$(MODULE_OBJ_DIR)/config.o: nd100x_types.h nd100x_protos.h
$(MODULE_OBJ_DIR)/nd100x.o: nd100x_types.h nd100x_protos.h

# Install the configuration file
install: all
	@mkdir -p $(BUILD_DIR)/etc
	cp $(SRC_DIR)/nd100x.conf $(BUILD_DIR)/etc/

# Clean module's build artifacts
clean:
	rm -f $(OBJS)
	rm -f $(MODULE_BIN)
	

.PHONY: all clean create_dirs install 

# Default target
.DEFAULT_GOAL := all