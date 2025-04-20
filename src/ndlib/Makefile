# Source directory (relative to this Makefile)
SRC_DIR := .

# Source files in dependency order
SRCS := log.c ndlib.c bpun.c aout.c

# Include paths
INCLUDES := -I$(SRC_DIR) -I../cpu

# NDLib module specific flags
CFLAGS += -D_GNU_SOURCE -DDEBUG -g -O0

# Module name
MODULE := ndlib

# Build paths - use parent's paths if available, otherwise set defaults
BUILD_DIR ?= ../../build
OBJ_DIR ?= $(BUILD_DIR)/obj
LIB_DIR ?= $(BUILD_DIR)/lib

# Build paths for this module
MODULE_OBJ_DIR := $(OBJ_DIR)/$(MODULE)
OBJS := $(SRCS:%.c=$(MODULE_OBJ_DIR)/%.o)
MODULE_LIB := $(LIB_DIR)/lib$(MODULE).a

# point this at wherever you built mkptypes
MKPTYPES    := ./../../tools/mkptypes/mkptypes

# Combine prototypes into cpu_protos.h
ndlib_protos.h: $(SRCS)
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
	@mkdir -p $(LIB_DIR)

# Build the library
all: | create_dirs ndlib_protos.h $(MODULE_LIB)

# Create the static library
$(MODULE_LIB): $(OBJS)
	$(AR) rcs $@ $^

# Pattern rule for object files
$(MODULE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Header dependencies
$(MODULE_OBJ_DIR)/log.o: ndlib_types.h ndlib_protos.h
$(MODULE_OBJ_DIR)/bpun.o: ndlib_types.h ndlib_protos.h
$(MODULE_OBJ_DIR)/ndlib.o: ndlib_types.h ndlib_protos.h

# Clean module's build artifacts
clean:
	rm -f $(OBJS)
	rm -f $(MODULE_LIB)


.PHONY: all clean create_dirs 

# Default target
.DEFAULT_GOAL := all

