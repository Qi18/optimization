__kernel void hello_kernel(__global char ***a,
	__global uchar **b,
	__global char ***result)
{
	int gid = get_global_id(0);

	int8 **data = a[gid];

	uint8 *index = b[gid];

    for (int i = 0; i < 8; i++) {
        result[gid][i] = data[index[index]];
    }
}