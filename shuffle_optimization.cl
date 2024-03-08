__kernel void hello_kernel(__global const char *src, __global const uchar *index, __global char *result,
							const int B, const int L, const int M, const int K)
{
	/*
		m_a ia a B*L*K matrix
		b is a B*M vector
		result = m_b[b[size]]
	*/
	int b = get_global_id(0);
	int l = get_global_id(1);

	int src_offset = b * L * K + l * K;
	int index_offset = b * M;
	int res_offset = b * L * M + l * M;

	uchar16 index_vector = convert_uchar16(vload16(0, index + index_offset));
	char16 res;
	for (int i = 0; i < K / 16; i++) {
		//M一般为16
		char16 src_vector = convert_char16(vload16(0, src + src_offset + i * 16 * sizeof(char)));
		char16 valid_vector = (char16) (0);
		for (int j = 0; j < 16 ; j++) {
			if (index_vector[j] / 16 == i) valid_vector[j] = -1;
		}
		//valid_vector.s0 = -1;
 		char16 temp = shuffle(src_vector, index_vector);
		res = select(res, temp, valid_vector);
	}
	vstore16(res, 0, result + res_offset);
}