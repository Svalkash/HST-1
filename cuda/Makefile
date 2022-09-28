CC = gcc
CUC = nvcc -ccbin g++
CFLAGS = -std=gnu99
CUFLAGS = -m64 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_70,code=sm_70 -gencode arch=compute_75,code=sm_75 -gencode arch=compute_80,code=sm_80 -gencode arch=compute_86,code=sm_86 -gencode arch=compute_86,code=compute_86
_DEPS = lib.h
BUILD_DIR = .
SRC_DIR = src
LIB_DIR = ../common
DEPS = $(patsubst %,$(LIB_DIR)/%,$(_DEPS))
HOST = $(BUILD_DIR)/host.exe
CLI = $(BUILD_DIR)/client.exe

.PHONY: all cli host clean

all: $(CLI) $(HOST)

$(HOST): $(LIB_DIR)/host.c $(DEPS)
	$(CC) $(CFLAGS) $< -o $@

$(CLI): $(SRC_DIR)/client.cu $(DEPS)
	$(CUC) $(CUFLAGS) $< -o $@

#---------------------------------------------------------------

ADDR = 127.0.0.1
PORT = 12345

SIZEMB = 256

cli:
	./client.exe $(ADDR) $(PORT)
host:
	mkdir -p logs
	./host.exe $(PORT) $(SIZEMB)


#---------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)/*.exe
	rm -f logs/*

#---------------------------------------------------------------