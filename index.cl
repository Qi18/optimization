__kernel void hello_kernel(__global const char *src, __global const uchar *index, __global char *result,
							const int B, const int L, const int M, const int K)
{
	/*
		src ia a B*L*K matrix
		index is a B*M vector
		result = m_b[b[size]]
	*/
	int b = get_global_id(0);
	int l = get_global_id(1);

	for (int i = 0; i < M; i++) {
		result[b * L * M + l * M + i] = src[b * L * K + l * K + index[b * M + i]];
	}
}