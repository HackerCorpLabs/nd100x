# mkptypes — portable build
#
# Respects CC/CFLAGS from the environment so MinGW, clang, cross-compilers
# all work. Emits mkptypes.exe on Windows, mkptypes elsewhere.

CC      ?= gcc
CFLAGS  ?= -O2

ifeq ($(OS),Windows_NT)
    PROG = mkptypes.exe
else
    PROG = mkptypes
endif

MANPAGE = mkptypes.1
RM      = rm -f

all: $(PROG)

$(PROG): mkptypes.c mkptypes.h
	$(CC) $(CFLAGS) -o $(PROG) mkptypes.c

clean:
	-$(RM) mkptypes.o mkptypes mkptypes.exe

realclean: clean
	-$(RM) report core
