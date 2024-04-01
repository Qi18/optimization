#define TS 16
__kernel void hello_kernel(__global const char *A, __global const char *B, __global char *C,
							const int N, const int K, const int M)
{
	const int row = get_local_id(0); // Local row ID (max: TS)
    const int col = get_local_id(1); // Local col ID (max: TS)
    const int globalRow = TS * get_group_id(0) + row; // Row ID of C (0..N)
    const int globalCol = TS * get_group_id(1) + col; // Col ID of C (0..M)

    __local float Asub[TS][TS];
    __local float Bsub[TS][TS];

    int acc = 0;

    const int numTiles = K / TS;
    for (int t = 0; t < numTiles; t++) {

        const int tiledRow = TS * t + row;
        const int tiledCol = TS * t + col;
        Asub[row][col] = A[globalRow * K + tiledCol];
        Bsub[row][col] = B[tiledRow * M + globalCol];

        barrier(CLK_LOCAL_MEM_FENCE);

        for (int k = 0; k < TS; k++) {
            acc += Asub[row][k] * Bsub[k][col];
        }

        //printf("acc = %d\n",acc);
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    C[globalRow*M + globalCol] = acc;
}

