#define uint64_t unsigned long long

extern "C" {
    void gpuProcess(double *vect, double *avg_vect, uint64_t local_blocks, uint64_t M);
}