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
  signed char ***res = create3D(B, L, M);
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < B; i++) {
    for (int k = 0; k < L; k++) {
      for (int j = 0; j < M; j++) {
        if (index[i][j] < 0 || index[i][j] >= K) continue;
        res[i][k][j] = src[i][k][index[i][j]];
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU time: " << nanoseconds << " ns" << std::endl;
  return res;
}

void print(__m128i data) {
  int8_t *ptr = (int8_t *)(&data);
  for (int i = 0; i < 16; i++) {
    std::cout << (int)ptr[i] << " ";
  }
  std::cout << std::endl;
}

signed char*** calIndexTimeByPSHUFL(signed char ***src, uint8_t **index, int B, int K, int L, int M = 16) {
  // src:[B, L, M] + index:[B, M] -> [B, L, M]
  // M == 16
  signed char ***res = create3D(B, L, M);
  long long val = 0;
  // auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < B; i++) {
    __m128i* vdata_index_masks = new __m128i[K / M];
    // __m256i vdata_index = _mm256_load_si256(reinterpret_cast<const __m256i*>(index[i]));
    __m128i vdata_index = _mm_loadu_si128(reinterpret_cast<const __m128i*>(index[i]));
    __m128i vdata_index_remain;
    for (int l = 0; l < M; l++) ((char *)(&vdata_index_remain))[l] = (((char *)(&vdata_index))[l] % M + M) % M;
    for (int k = 0; k < K / M; k++) {
      for (int l = 0; l < M; l++) ((char *)(&(vdata_index_masks[k])))[l] = ((char *)(&vdata_index))[l] / M == k ? -1 : 0;//之后会做逻辑运算
    }
    auto start = std::chrono::high_resolution_clock::now();
    // if (i == 8) print(vdata_index);
    // if (i == 8) print(vdata_index_remain);
    for (int j = 0; j < L; j++) {
      __m128i tbl_res = _mm_setzero_si128();
      for (int k = 0; k < K / M; k++) {
        __m128i vdata = _mm_load_si128(reinterpret_cast<const __m128i*>(&src[i][j][k * M]));
        // if (i == 8 && j == 0) print(vdata);
        // for (int l = 0; l < M; l++) vdata_ptr[l] = src[i][k * 16 + l][j];//这里不知道转置能不能优化
        // __m256i xmm_result = _mm256_shuffle_epi8(vdata, vdata_index);
        __m128i tbl_temp_res = _mm_shuffle_epi8(vdata, vdata_index_remain);
        // if (i == 8 && j == 0) print(tbl_temp_res);
        // and操作，过滤无效的向量元素
        __m128i tbl_temp_res_masked = _mm_and_si128(tbl_temp_res, vdata_index_masks[k]);
        // if (i == 8 && j == 0) print(tbl_temp_res_masked);
        //or操作，合并成一个结果向量
        tbl_res = _mm_or_si128(tbl_res, tbl_temp_res_masked);
        // if (i == 8 && j == 0) print(tbl_res);
      }
      _mm_store_si128(reinterpret_cast<__m128i*>(&res[i][j][0]), tbl_res);
      // if (i == 8 && j == 0) {
      //   for (int l = 0; l < M; l++) {
      //     std::cout << (int)res[i][j][l] << " ";
      //   }}
      // for (int l = 0; l < M; l++) {
      //   res[i][l][j] = tbl_res_ptr[l];
      // }
    }
    auto end = std::chrono::high_resolution_clock::now();
    val += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  }
  // auto end = std::chrono::high_resolution_clock::now();
  // auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  auto nanoseconds = val;
  std::cout << "CPU tbl time: " << val << " ns" << std::endl;
  // std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}

int main() {
    // M为8或16
    //总time K <= 16 * 8 shuffle有优化
    //总time K <= 16 * 9 shuffle有优化
    int B = 100, K = 16 * 10, L = 100, M = 16;
    signed char ***src = create3D(B, L, K);
    uint8_t **index = create2D(B, M);
    init3D(src, B, L, K);
    init2D(index, B, M, K);
    signed char ***res1 = calIndexTime(src, index, B, K, L, M);
    signed char ***res2 = calIndexTimeByPSHUFL(src, index, B, K, L, M);
    // print3D(src, B, K, L);
    // print2D(index, B, M);
    // print3D(res1, B, M, L);
    // print3D(res2, B, M, L);
    isEqual(res1, res2, B, L, M);
    delete3D(src, B, L);
    delete2D(index, B);
    delete3D(res1, B, L);
    delete3D(res2, B, L);
    return 0;
}