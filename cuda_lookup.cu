#include <stdio.h>
#include <cuda_runtime.h>

#define BLOCK_SIZE 32  // 定义每个块的大小

__global__ void lookupCUDA(float *output, float *source, int *index, int L, int K) {
    int row = blockIdx.x;
    int group = blockIdx.y;
    int tid = threadIdx.x;  // idx in index
    int wrap_start_idx = blockIdx.y * blockDim.x;
    int wrap_end_idx = (blockIdx.y + 1) * blockDim.x;
    if (wrap_start_idx <= index[tid] && index[tid] < wrap_end_idx) {
        output[blockIdx.x * blockDim.x + tid] = source[blockIdx.x * K + index[tid]];
    }
}

int main() {
    for (int L = 64; L <= 1024; L += 64) {
        for (int x = 1; x <= 4; x++) {
            int K = BLOCK_SIZE * x;    // source的行数，32的倍数
            int M = BLOCK_SIZE;    // index的大小

            // 分配主机内存
            float *h_source = (float *)malloc(K * L * sizeof(float));
            int *h_index = (int *)malloc(M * sizeof(int));
            float *h_output = (float *)malloc(M * L * sizeof(float));

            // 初始化source和index
            for (int i = 0; i < K * L; ++i) {
                h_source[i] = (float)i;
            }
            for (int i = 0; i < M; ++i) {
                h_index[i] = i * 2;  // 假设简单地查找前M个元素
            }
            // for (int i = 0; i < L; ++i) {
            //     for (int j = 0; j < K; ++j) {
            //         printf("%f ", h_source[i * K + j]);
            //     }
            //     printf("\n");
            // }
            //     printf("\n");
            // for (int i = 0; i < L; ++i) {
            //     for (int j = 0; j < M; ++j) {
            //         printf("%f ", h_source[i * K + h_index[j]]);
            //     }
            //     printf("\n");
            // }

            // 分配设备内存
            float *d_source, *d_output;
            int *d_index;
            cudaMalloc((void **)&d_source, L * K * sizeof(float));
            cudaMalloc((void **)&d_index, M * sizeof(int));
            cudaMalloc((void **)&d_output, L * M * sizeof(float));

            // 将数据从主机复制到设备
            cudaMemcpy(d_source, h_source, K * L * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(d_index, h_index, M * sizeof(int), cudaMemcpyHostToDevice);

            // 定义块的大小
            dim3 numBlocks(L, K / BLOCK_SIZE);
            dim3 threadsPerBlock(BLOCK_SIZE);

            // 启动计时
            cudaEvent_t start, stop;
            cudaEventCreate(&start);
            cudaEventCreate(&stop);
            cudaEventRecord(start);


            // 启动核函数
            lookupCUDA<<<numBlocks, threadsPerBlock>>>(d_output, d_source, d_index, L, K);

            // 停止计时
            cudaEventRecord(stop);
            cudaEventSynchronize(stop);

            float milliseconds = 0;
            cudaEventElapsedTime(&milliseconds, start, stop);
            printf("[L=%d, K=%d, M=%d] Kernel execution time: %f ms\n", L, K, M, milliseconds);

            // 将结果从设备复制回主机
            cudaMemcpy(h_output, d_output, M * L * sizeof(float), cudaMemcpyDeviceToHost);

            // 验证结果
            bool correct = true;
            for (int i = 0; i < L; ++i) {
                for (int j = 0; j < M; ++j) {  // h_source[i * K + h_index[j]]
                    // printf("(%f %f)", h_output[i * M + j], h_source[i * K + h_index[j]]);
                    if (h_output[i * M + j] != h_source[i * K + h_index[j]]) {
                        correct = false;
                    }
                }
                // printf("\n");
            }

            if (correct) {
                printf("Lookup is correct.\n");
            } else {
                printf("Lookup is incorrect.\n");
            }

            // 释放内存
            free(h_source);
            free(h_index);
            free(h_output);
            cudaFree(d_source);
            cudaFree(d_index);
            cudaFree(d_output);
            cudaEventDestroy(start);
            cudaEventDestroy(stop);
        }
    }


    

    return 0;
}
