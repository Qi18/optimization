#include <immintrin.h>
#include <vector>
#include <time.h>
#include <list>
#include "Matrix.h"
#include <iostream>
// #include "sse2neon.h"
// #include <emmintrin.h>

signed char*** calIndexTime(signed char ***src, uint8_t **index, int B, int K, int L, int M) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < L; k++) {
        if (index[i][j] < 0 || index[i][j] >= K) continue;
        res[i][j][k] = src[i][index[i][j]][k];
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU time: " << nanoseconds << " ns" << std::endl;
  return res;
}

signed char*** calIndexTimeByPSHUFL(signed char ***src, uint8_t **index, int B, int K, int L, int M) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  // M == 16
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    __m128i* vdata_index_masks = new __m128i[K / M];
    // __m256i vdata_index = _mm256_load_si256(reinterpret_cast<const __m256i*>(index[i]));
    __m128i vdata_index = _mm_loadu_si128(reinterpret_cast<const __m128i*>(index[i]));
    __m128i vdata_index_remain;
    for (int l = 0; l < M; l++) ((char *)(&vdata_index_remain))[l] = ((char *)(&vdata_index_remain))[l] % M;
    for (int k = 0; k < K / M; k++) {
      for (int l = 0; l < M; l++) ((char *)(&(vdata_index_masks[k])))[l] = ((char *)(&vdata_index))[l] / M == k ? -1 : 0;//之后会做逻辑运算
    }
    for (int j = 0; j < L; j++) {
      __m128i tbl_res;
      for (int l = 0; l < M; l++) ((char *)(&tbl_res))[l] = 0;
      for (int k = 0; k < K / M; k++) {
        __m128i vdata;
        for (int l = 0; l < M; l++) ((char *)(&vdata))[l] = src[i][k * 16 + l][j];//这里不知道转置能不能优化
        // __m256i xmm_result = _mm256_shuffle_epi8(vdata, vdata_index);
        __m128i tbl_temp_res = _mm_shuffle_epi8(vdata, vdata_index_remain);
        //and操作，过滤无效的向量元素
        __m128i tbl_temp_res_masked = _mm_and_si128(tbl_temp_res, vdata_index_masks[k]);
        //or操作，合并成一个结果向量
        tbl_res = _mm_or_si128(tbl_res, tbl_temp_res_masked);
      }
      // for (int l = 0; l < 8; l++) if (vdata_index[l] != -1) res[i][l][j] = tbl_2[l];
      for (int l = 0; l < M; l++) res[i][l][j] = ((char *)(&tbl_res))[l];
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}

int main() {
    // M为8或16
    int B = 1000, K = 16, L = 100, M = 16;
    signed char ***src = create3D(B, K, L);
    uint8_t **index = create2D(B, M);
    init3D(src, B, K, L);
    init2D(index, B, M, K);
    signed char ***res1 = calIndexTime(src, index, B, K, L, M);
    signed char ***res2 = calIndexTimeByPSHUFL(src, index, B, K, L, M);
    // print3D(src, B, K, L);
    // print2D(index, B, M);
    // print3D(res1, B, M, L);
    // print3D(res2, B, M, L);
    // isEqual(res1, res2, B, M, L);
    delete3D(src, B, K);
    delete2D(index, B);
    delete3D(res1, B, M);
    delete3D(res2, B, M);
    return 0;
}