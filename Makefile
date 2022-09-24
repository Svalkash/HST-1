CC = gcc
CFLAGS = -std=gnu99
_DEPS = lib.h
BUILD_DIR = .
SRC_DIR = src
DEPS = $(patsubst %,$(SRC_DIR)/%,$(_DEPS))
SERV = $(BUILD_DIR)/host.exe
CLI = $(BUILD_DIR)/client.exe

.PHONY: all cli host clean

all: $(CLI) $(SERV)

$(BUILD_DIR)/host.exe: $(SRC_DIR)/host.c $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/client.exe: $(SRC_DIR)/client.c $(DEPS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

#---------------------------------------------------------------

ADDR = 127.0.0.1
PORT = 12345

SIZEMB = 16

cli:
	./client.exe $(ADDR) $(PORT)
host:
	./host.exe $(PORT) $(SIZEMB)


#---------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)/*.exe
	rm -f logs/*

#---------------------------------------------------------------