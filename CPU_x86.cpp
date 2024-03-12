// #include <immintrin.h>
#include <vector>
#include <time.h>
#include <list>
#include "Matrix.h"
#include "sse2neon.h"
#include <iostream>

void shuffleBytes(const unsigned char* input, unsigned char* output, const unsigned char* mask) {
    __m128i xmm_input = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
    __m128i xmm_mask = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask));
    __m128i xmm_result = _mm_shuffle_epi8(xmm_input, xmm_mask);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output), xmm_result);
}

signed char*** calIndexTime(signed char ***src, uint8_t **index, int B, int K, int L, int M) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  clock_t t1,t2;
  float val = 0.0;
  t1 = clock();
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < L; k++) {
        if (index[i][j] < 0 || index[i][j] >= K) continue;
        t2 = clock();
        res[i][j][k] = src[i][index[i][j]][k];
        val += (float)(clock()-t2);
      }
    }
  }
  t2 = clock();
  printf("cpu 总赋值t = %.8f\n",val/CLOCKS_PER_SEC);
  printf("cpu 一次赋值t = %.8f\n",(val / (B * M * L))/CLOCKS_PER_SEC);
  printf("cpu 总t = %.8f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
  return res;
}

signed char*** calIndexTimeByTBL(signed char ***src, uint8_t **index, int B, int K, int L, int M) {
  clock_t t1,t2;
  t1 = clock();
  float val = 0.0;
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    for (int k = 0; k < K / M; k++) {
      __m128i vdata, vdata_index;
      vdata_index = _mm_loadu_si128(reinterpret_cast<const __m128i*>(index[i]));
      for (int l = 0; l < M; l++) {
        cout << (int)vdata_index[l] << " ";
        vdata_index[l] = vdata_index[l] / M == k ? vdata_index[l] % M : -1;
        cout << (int)vdata_index[l] << endl;
        }
      for (int j = 0; j < L; j++) {
        for (int l = 0; l < M; l++) vdata[l] = src[i][k * M + l][j];
        t2 = clock();
        __m128i xmm_result = _mm_shuffle_epi8(vdata, vdata_index);
        val += (float)(clock()-t2);
        for (int l = 0; l < M; l++) if (vdata_index[l] != -1) res[i][l][j] = xmm_result[l];
      }
      // vst1_s8(&(res[i][j][0]), tbl_2);
    }
  }
  t2 = clock();
  printf("cpu tbl 总赋值t = %.8f\n",val/CLOCKS_PER_SEC);
  printf("cpu tbl 一次赋值t = %.8f\n",(val * M / (B * K * L))/CLOCKS_PER_SEC);
  printf("cpu tbl 总t = %.8f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
  return res;
}

int main() {
    // M为8或16
    int B = 1, K = 16, L = 1, M = 16;
    signed char ***src = create3D(B, K, L);
    uint8_t **index = create2D(B, M);
    init3D(src, B, K, L);
    init2D(index, B, M, K);
    signed char ***res2 = calIndexTime(src, index, B, K, L, M);
    signed char ***res1 = calIndexTimeByTBL(src, index, B, K, L, M);
    isEqual(res1, res2, B, M, L);
    print3D(src, B, K, L);
    print2D(index, B, M);
    print3D(res2, B, M, L);
    print3D(res1, B, M, L);
    delete3D(src, B, K);
    delete2D(index, B);
    delete3D(res2, B, M);
    delete3D(res1, B, M);
    return 0;
}