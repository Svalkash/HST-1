#include "client_cuda.hpp"

__global__ void AVG(const double *from, double *to, uint64_t N, uint64_t M) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int step = gridDim.x * blockDim.x;
    //printf("N %ld, M %ld\n", N, M);
    for (long long ti = tid; ti < N / M; ti += step)
    { // assuming N=M*x
        double sum = 0;
        for (long long fi = ti * M; fi < (ti + 1) * M; ++fi)
            sum += from[fi];
        to[ti] = sum / M;
    }
}

void gpuProcess(double *vect, double *avg_vect, uint64_t local_blocks, uint64_t M) {
    double *cu_vect, *cu_avg_vect;

    // Allocate device memory
    cudaMalloc((void**)&cu_vect, local_blocks * M * sizeof(double));
    cudaMalloc((void**)&cu_avg_vect, local_blocks * sizeof(double));
    // Transfer data from host to device memory
    cudaMemcpy(cu_vect, vect, sizeof(double) * local_blocks * M, cudaMemcpyHostToDevice);
    // Executing kernel 
    int cuBlock_size = 1024;
    int grid_size = (local_blocks + cuBlock_size) / cuBlock_size;
    AVG<<<grid_size,cuBlock_size>>>(cu_vect, cu_avg_vect, local_blocks * M, M);
    cudaDeviceSynchronize();
    // Transfer data back to host memory
    cudaMemcpy(avg_vect, cu_avg_vect, sizeof(double) * local_blocks, cudaMemcpyDeviceToHost);
    // Deallocate device memory
    cudaFree(cu_vect);
    cudaFree(cu_avg_vect);
}