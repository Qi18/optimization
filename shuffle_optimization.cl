__kernel void hello_kernel(__global const int8_t ***a,
	__global const uint8_t **b,
	__global int8_t ***result)
{
	int gid = get_global_id(0);

	int8_t **data = a[gid];

	uint8_t *index = b[gid];
 
	result[gid] = shuffle(data, index);
}