# reth-tap - bridge a gateway ethernet segment onto a host TAP device.
#
# Linux only: TAP is a Linux interface. No dependencies beyond libc, on
# purpose - this has to be buildable next to the gateway without dragging a
# toolchain in.
#
#   make          build the bridge
#   make test     framing checks, no privilege and no network needed
#   make test-gw  end-to-end against a throwaway gateway (needs node)
#   make check    both

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c99 -D_GNU_SOURCE
BIN      = reth-tap
TESTS    = test-framing test-gateway

all: $(BIN)

$(BIN): reth-tap.c
	$(CC) $(CFLAGS) -o $@ $<

# Both tests #include reth-tap.c rather than linking it: its pumps are static,
# and making them extern purely to be tested would change the program to suit
# the test.
test-framing: test-framing.c reth-tap.c
	$(CC) $(CFLAGS) -o $@ $<

test-gateway: test-gateway.c reth-tap.c
	$(CC) $(CFLAGS) -o $@ $<

test: test-framing
	./test-framing

test-gw: test-gateway
	./test-gateway.sh

check: test test-gw

clean:
	rm -f $(BIN) $(TESTS)

.PHONY: all test test-gw check clean
