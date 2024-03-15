#include <stdio.h>
#include <cuda_runtime.h>

#define BLOCK_SIZE 16  // 定义每个块的大小

__global__ void matrixMulCUDA(float *C, float *A, float *B, int M, int K, int N) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < M && col < N) {
        float sum = 0.0;
        for (int i = 0; i < K; ++i) {
            sum += A[row * K + i] * B[i * N + col];
        }
        C[row * N + col] = sum;
    }
}

int main() {
    for(int M = 64; M <= 1024; M += 64) {
        for(int N = 64; N <= 1024; N += 64) {
            for(int K = 64; K <= 1024; K += 64) {
                // 分配主机内存
                float *h_A = (float *)malloc(M * K * sizeof(float));
                float *h_B = (float *)malloc(K * N * sizeof(float));
                float *h_C = (float *)malloc(M * N * sizeof(float));

                // 初始化矩阵A和矩阵B
                for (int i = 0; i < M * K; ++i) {
                    h_A[i] = 1.0f;
                }
                for (int i = 0; i < K * N; ++i) {
                    h_B[i] = 2.0f;
                }

                // 分配设备内存
                float *d_A, *d_B, *d_C;
                cudaMalloc((void **)&d_A, M * K * sizeof(float));
                cudaMalloc((void **)&d_B, K * N * sizeof(float));
                cudaMalloc((void **)&d_C, M * N * sizeof(float));

                // 将矩阵A和矩阵B从主机复制到设备
                cudaMemcpy(d_A, h_A, M * K * sizeof(float), cudaMemcpyHostToDevice);
                cudaMemcpy(d_B, h_B, K * N * sizeof(float), cudaMemcpyHostToDevice);

                // 定义网格和块的大小
                dim3 threadsPerBlock(BLOCK_SIZE, BLOCK_SIZE);
                dim3 blocksPerGrid((N + BLOCK_SIZE - 1) / BLOCK_SIZE, (M + BLOCK_SIZE - 1) / BLOCK_SIZE);

                // 启动计时
                cudaEvent_t start, stop;
                cudaEventCreate(&start);
                cudaEventCreate(&stop);
                cudaEventRecord(start);
                // 启动核函数
                matrixMulCUDA<<<blocksPerGrid, threadsPerBlock>>>(d_C, d_A, d_B, M, K, N);

                cudaEventRecord(stop);
                cudaEventSynchronize(stop);

                float milliseconds = 0;
                cudaEventElapsedTime(&milliseconds, start, stop);
                printf("[M=%d, K=%d, N=%d] Kernel execution time: %f ms\n", M, K, N, milliseconds);

                // 将结果从设备复制回主机
                cudaMemcpy(h_C, d_C, M * N * sizeof(float), cudaMemcpyDeviceToHost);

                // 验证结果
                bool correct = true;
                for (int i = 0; i < M * N; ++i) {
                    if (h_C[i] != K * 2) {
                        correct = false;
                        break;
                    }
                }

                if (correct) {
                    printf("Matrix multiplication is correct.\n");
                } else {
                    printf("Matrix multiplication is incorrect.\n");
                }

                // 释放内存
                free(h_A);
                free(h_B);
                free(h_C);
                cudaFree(d_A);
                cudaFree(d_B);
                cudaFree(d_C);

            }
        }

    }

    return 0;
}
