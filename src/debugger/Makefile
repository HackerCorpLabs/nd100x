# Source directory (relative to this Makefile)
SRC_DIR := .

# Source files in dependency order
SRCS := debugger.c

# CPU module specific flags
CFLAGS += -D_GNU_SOURCE -DDEBUG

# NO DEBUG (commented out)
#CFLAGS = -Wall -O3 -pg -fno-aggressive-loop-optimizations -ggdb

# Debug mode (commented out as we're using the top flags)
#CFLAGS = -Wall -O0 -fno-aggressive-loop-optimizations -ggdb -DDEBUG

# Module name (used for build output paths)
MODULE := debugger

# Build paths - use parent's paths if available, otherwise set defaults
BUILD_DIR ?= ../../build
OBJ_DIR ?= $(BUILD_DIR)/obj
LIB_DIR ?= $(BUILD_DIR)/lib


# Build paths for this module
MODULE_OBJ_DIR := $(OBJ_DIR)/$(MODULE)
OBJS := $(addprefix $(MODULE_OBJ_DIR)/,$(SRCS:.c=.o))
MODULE_LIB := $(LIB_DIR)/lib$(MODULE).a

# point this at wherever you built mkptypes
MKPTYPES    := ./../../tools/mkptypes/mkptypes


# Include LibDAP headers
INCLUDES += -I$(LIBDAP_INCLUDE_DIR)

# Link with LibDAP
LDLIBS += -L$(LIBDAP_LIB_DIR) -ldap

# Combine prototypes into debugger_protos.h
debugger_protos.h: $(SRCS)
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
all: | create_dirs debugger_protos.h $(MODULE_LIB)

# Create the static library
$(MODULE_LIB): $(OBJS)
	$(AR) rcs $@ $^

# Pattern rule for object files
$(MODULE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(LDFLAGS) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Header dependencies
$(MODULE_OBJ_DIR)/debugger.o: debugger_protos.h debugger.h

# Clean module's build artifacts
clean:
	rm -f $(OBJS)
	rm -f $(MODULE_LIB)

.PHONY: all clean create_dirs 


# Default target
.DEFAULT_GOAL := all
