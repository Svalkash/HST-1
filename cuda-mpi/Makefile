CC = gcc
MPICXX = mpicxx
CUC = nvcc -ccbin g++
CUFLAGS = -m64 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_70,code=sm_70 -gencode arch=compute_75,code=sm_75 -gencode arch=compute_80,code=sm_80 -gencode arch=compute_86,code=sm_86 -gencode arch=compute_86,code=compute_86
LDFLAGS = -L /usr/local/cuda/lib64 -lcudart
_DEPS = lib.h
SRC_DIR = src
LIB_DIR = ../common
DEPS = $(patsubst %,$(LIB_DIR)/%,$(_DEPS))

.PHONY: all cli host clean

all: host.exe client.exe

host.exe: $(LIB_DIR)/host.c $(DEPS)
	$(CC) $< -o $@

client_mpi.o: src/client_mpi.cpp $(DEPS) src/client_cuda.hpp
#	export PATH=${PATH}:/usr/lib64/openmpi/bin:/usr/lib64/mpich-3.2/bin
	$(MPICXX) -o $@ -c $<
client_cuda.o: src/client_cuda.cu src/client_cuda.hpp
	$(CUC) $(CUFLAGS) -o $@ -c $<
client.exe: client_mpi.o client_cuda.o
	$(MPICXX) $(LDFLAGS) $^ -o $@

#---------------------------------------------------------------

ADDR = 127.0.0.1
PORT = 12346

SIZEMB = 64

MPISLOTS = 2

cli:
	./client.exe $(ADDR) $(PORT)

host:
	mkdir -p logs
	./host.exe $(PORT) $(SIZEMB)


#---------------------------------------------------------------

clean:
	rm -rf client_mpi.o client_cuda.o client.exe host.exe
	rm -f logs/*

#---------------------------------------------------------------