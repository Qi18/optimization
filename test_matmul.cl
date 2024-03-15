__kernel void hello_kernel(__global const char *A, __global const char *B, __global char *result,
							const int N, const int K, const int M)
{
	int row = get_global_id(0);
	int col = get_global_id(1);

	int A_offset = row * K;
	int res_offset = row * M + col;
	for (int i = 0; i < K; i++) {
        result[res_offset] += A[A_offset + i] * B[i * M + col];        
	}
}