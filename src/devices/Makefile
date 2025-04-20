# Source directory (relative to this Makefile)
SRC_DIR := .

# Source files in dependency order
SRCS := device.c \
        devicemanager.c \
        floppy/deviceFloppyPIO.c \
        floppy/deviceFloppyDMA.c \
        papertape/devicePapertape.c \
        smd/deviceSMD.c \
        smd/diskSMD.c \
        rtc/deviceRTC.c \
        panel/panel.c \
        terminal/deviceTerminal.c

# Include paths for all device subdirectories
INCLUDES := -I$(SRC_DIR) \
           -I$(SRC_DIR)/common \
           -I$(SRC_DIR)/terminal \
           -I$(SRC_DIR)/floppy \
           -I$(SRC_DIR)/papertape \
           -I$(SRC_DIR)/smd \
           -I$(SRC_DIR)/rtc \
           -I$(SRC_DIR)/panel \
           -I../cpu \
           -I../ndlib

# Devices module specific flags
CFLAGS += -D_GNU_SOURCE -DDEBUG

# Module name
MODULE := devices

# Build paths - use parent's paths if available, otherwise set defaults
BUILD_DIR ?= ../../build
OBJ_DIR ?= $(BUILD_DIR)/obj
LIB_DIR ?= $(BUILD_DIR)/lib


# Build paths for this module
MODULE_OBJ_DIR := $(OBJ_DIR)/$(MODULE)
OBJS := $(SRCS:%.c=$(MODULE_OBJ_DIR)/%.o)
MODULE_LIB := $(LIB_DIR)/lib$(MODULE).a

# Required libraries
LIBS := -L$(LIB_DIR) -lndlib

# point this at wherever you built mkptypes
MKPTYPES    := ./../../tools/mkptypes/mkptypes

# Combine prototypes into cpu_protos.h
devices_protos.h: $(SRCS)
	@echo "Creating combined prototype header $@"
	@echo "/* AUTO-GENERATED FILE. DO NOT EDIT! */" > $@
	@for src in $(SRCS); do \
		echo "/* Prototypes from $$src */" >> $@; \
		$(MKPTYPES) $$src >> $@; \
		echo "" >> $@; \
	done


# Create necessary directories
create_dirs:
	@mkdir -p $(MODULE_OBJ_DIR)/common
	@mkdir -p $(MODULE_OBJ_DIR)/floppy
	@mkdir -p $(MODULE_OBJ_DIR)/papertape
	@mkdir -p $(MODULE_OBJ_DIR)/smd
	@mkdir -p $(MODULE_OBJ_DIR)/rtc
	@mkdir -p $(MODULE_OBJ_DIR)/panel
	@mkdir -p $(MODULE_OBJ_DIR)/terminal

# Build the library
all: | create_dirs $(PIS) $(MODULE_LIB)

# Create the static library
$(MODULE_LIB): $(OBJS)
	$(AR) rcs $@ $^

# Pattern rule for object files
$(MODULE_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

# Header dependencies
$(MODULE_OBJ_DIR)/device.o: devices_protos.h devices_types.h    
$(MODULE_OBJ_DIR)/devicemanager.o: devices_protos.h devices_types.h    
$(MODULE_OBJ_DIR)/floppy/deviceFloppyPIO.o: floppy/deviceFloppyPIO.h devices_protos.h
$(MODULE_OBJ_DIR)/floppy/deviceFloppyDMA.o: floppy/deviceFloppyDMA.h devices_protos.h
$(MODULE_OBJ_DIR)/papertape/devicePapertape.o: papertape/devicePapertape.h devices_protos.h
$(MODULE_OBJ_DIR)/smd/deviceSMD.o: smd/deviceSMD.h smd/diskSMD.h devices_protos.h
$(MODULE_OBJ_DIR)/smd/diskSMD.o: smd/diskSMD.h
$(MODULE_OBJ_DIR)/rtc/deviceRTC.o: rtc/deviceRTC.h devices_protos.h
$(MODULE_OBJ_DIR)/panel/panel.o: panel/panel.h devices_protos.h
$(MODULE_OBJ_DIR)/terminal/deviceTerminal.o: terminal/deviceTerminal.h devices_protos.h

# Clean module's build artifacts
clean:
	rm -f $(OBJS)
	rm -f $(MODULE_LIB)

.PHONY: all clean create_dirs 

# Default target
.DEFAULT_GOAL := all
