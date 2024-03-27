#define TS 16
__kernel void hello_kernel(__global const char *A, __global const char *B, __global char *C,
							const int N, const int K, const int M)
{
	const int globalRow = get_global_id(0); 
    const int globalCol = get_global_id(1); 
    const int row = get_local_id(0); 
    const int col = get_local_id(1); 

    __local float Asub[TS][TS];
    __local float Bsub[TS][TS];

    float acc = 0.0f;

    for (int t = 0; t < K/TS; t++) {
        Asub[row][col] = A[(globalRow * K) + (t*TS + col)];
        Bsub[row][col] = B[(t*TS + row) * N + globalCol];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (int k = 0; k < TS; k++) {
            acc += Asub[row][k] * Bsub[k][col];
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    C[globalRow * N + globalCol] = acc;
}