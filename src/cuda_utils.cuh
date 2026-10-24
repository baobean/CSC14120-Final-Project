/*
 * cuda_utils.cuh - CUDA helpers shared by all GPU versions.
 * Authors: NDB
 * External sources: CHECK macro follows the common pattern from the course demos
 *                   and the CUDA C Programming Guide (error handling section).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * Stage 2 adds the cudaEvent stage timers here.
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>

#define CHECK(call)                                                           \
    do {                                                                      \
        cudaError_t err_ = (call);                                            \
        if (err_ != cudaSuccess) {                                            \
            fprintf(stderr, "CUDA error %s:%d: %s\n", __FILE__, __LINE__,     \
                    cudaGetErrorString(err_));                                \
            exit(EXIT_FAILURE);                                               \
        }                                                                     \
    } while (0)

// Prints the GPU name (the spec asks for it with every result).
// Does not abort when there is no GPU, so V0 can still run.
inline void print_gpu_name() {
    int count = 0;
    if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) {
        printf("GPU: no CUDA device found\n");
        return;
    }
    cudaDeviceProp prop;
    CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("GPU: %s (compute %d.%d, %.1f GB)\n", prop.name, prop.major, prop.minor,
           prop.totalGlobalMem / (1024.0 * 1024.0 * 1024.0));
}
