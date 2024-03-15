#include <immintrin.h>
#include <vector>
#include <time.h>
#include <list>
#include "Matrix.h"
// #include "sse2neon.h"
#include <iostream>
// #include <emmintrin.h>

// void shuffleBytes(const unsigned char* input, unsigned char* output, const unsigned char* mask) {
//     __m128i xmm_input = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
//     __m128i xmm_mask = _mm_loadu_si128(reinterpret_cast<const __m128i*>(mask));
//     __m128i xmm_result = _mm_shuffle_epi8(xmm_input, xmm_mask);
//     _mm_storeu_si128(reinterpret_cast<__m128i*>(output), xmm_result);
// }

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
  std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}

signed char*** calIndexTimeByPSHUFL(signed char ***src, uint8_t **index, int B, int K, int L, int M) {
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    for (int k = 0; k < K / M; k++) {
      __m256i vdata, vdata_index;
      // vdata_index = _mm_loadu_si128(reinterpret_cast<const __m128i*>(index[i]));
      vdata_index = _mm256_load_si256(reinterpret_cast<const __m256i*>(index[i]));
      // int temp_value = _mm_extract_epi8(vdata_index, 0);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 0);
      // temp_value = _mm_extract_epi8(vdata_index, 1);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 1);
      // temp_value = _mm_extract_epi8(vdata_index, 2);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 2);
      // temp_value = _mm_extract_epi8(vdata_index, 3);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 3);
      // temp_value = _mm_extract_epi8(vdata_index, 4);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 4);
      // temp_value = _mm_extract_epi8(vdata_index, 5);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 5);
      // temp_value = _mm_extract_epi8(vdata_index, 6);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 6);
      // temp_value = _mm_extract_epi8(vdata_index, 7);
      // _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 7);
      // if (M == 16) {
      //   temp_value = _mm_extract_epi8(vdata_index, 8);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 8);
      //   temp_value = _mm_extract_epi8(vdata_index, 9);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 9);
      //   temp_value = _mm_extract_epi8(vdata_index, 10);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 10);
      //   temp_value = _mm_extract_epi8(vdata_index, 11);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 11);
      //   temp_value = _mm_extract_epi8(vdata_index, 12);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 12);
      //   temp_value = _mm_extract_epi8(vdata_index, 13);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 13);
      //   temp_value = _mm_extract_epi8(vdata_index, 14);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 14);
      //   temp_value = _mm_extract_epi8(vdata_index, 15);
      //   _mm_insert_epi8(vdata_index, temp_value / M == k ? temp_value % M : -1, 15);
      // }`
      for (int l = 0; l < M; l++) {
         ((char *)(&vdata_index))[l] = ((char *)(&vdata_index))[l] / M == k ? ((char *)(&vdata_index))[l] % M : -1;
        cout << (int)((char *)(&vdata_index))[l] << " ";
        // cout << (int)index[i][l] << " ";
      }
      cout << endl;
      for (int j = 0; j < L; j++) {
        for (int l = 0; l < M; l++) {
          ((char *)(&vdata))[l] = src[i][k * M + l][j];
          // cout << (int)src[i][k * M + l][j] << " ";
          cout << (int)((char *)(&vdata))[l] << " ";
        }
        // _mm_insert_epi8(vdata, src[i][k * M + 0][j], 0);
        // _mm_insert_epi8(vdata, src[i][k * M + 1][j], 1);
        // _mm_insert_epi8(vdata, src[i][k * M + 2][j], 2);
        // _mm_insert_epi8(vdata, src[i][k * M + 3][j], 3);
        // _mm_insert_epi8(vdata, src[i][k * M + 4][j], 4);
        // _mm_insert_epi8(vdata, src[i][k * M + 5][j], 5);
        // _mm_insert_epi8(vdata, src[i][k * M + 6][j], 6);
        // _mm_insert_epi8(vdata, src[i][k * M + 7][j], 7);
        // if (M == 16) {
        //   _mm_insert_epi8(vdata, src[i][k * M + 8][j], 8);
        //   _mm_insert_epi8(vdata, src[i][k * M + 9][j], 9);
        //   _mm_insert_epi8(vdata, src[i][k * M + 10][j], 10);
        //   _mm_insert_epi8(vdata, src[i][k * M + 11][j], 11);
        //   _mm_insert_epi8(vdata, src[i][k * M + 12][j], 12);
        //   _mm_insert_epi8(vdata, src[i][k * M + 13][j], 13);
        //   _mm_insert_epi8(vdata, src[i][k * M + 14][j], 14);
        //   _mm_insert_epi8(vdata, src[i][k * M + 15][j], 15);
        // }
        cout << endl;
        __m256i xmm_result = _mm256_shuffle_epi8(vdata, vdata_index);
        // __m128i xmm_result = _mm_shuffle_epi8(vdata, vdata_index);
        cout << endl;
        // if (_mm_extract_epi8(vdata_index, 0) != -1) {
        //   res[i][0][j] = _mm_extract_epi8(xmm_result, 0);
        //   cout << (int)_mm_extract_epi8(xmm_result, 0) << endl;
        // }
        // if (_mm_extract_epi8(vdata_index, 1) != -1) res[i][1][j] = _mm_extract_epi8(xmm_result, 1);
        // if (_mm_extract_epi8(vdata_index, 2) != -1) res[i][2][j] = _mm_extract_epi8(xmm_result, 2);
        // if (_mm_extract_epi8(vdata_index, 3) != -1) res[i][3][j] = _mm_extract_epi8(xmm_result, 3);
        // if (_mm_extract_epi8(vdata_index, 4) != -1) res[i][4][j] = _mm_extract_epi8(xmm_result, 4);
        // if (_mm_extract_epi8(vdata_index, 5) != -1) res[i][5][j] = _mm_extract_epi8(xmm_result, 5);
        // if (_mm_extract_epi8(vdata_index, 6) != -1) res[i][6][j] = _mm_extract_epi8(xmm_result, 6);
        // if (_mm_extract_epi8(vdata_index, 7) != -1) res[i][7][j] = _mm_extract_epi8(xmm_result, 7);
        // if (M == 16) {
        //   if (_mm_extract_epi8(vdata_index, 8) != -1) res[i][8][j] = _mm_extract_epi8(xmm_result, 8);
        //   if (_mm_extract_epi8(vdata_index, 9) != -1) res[i][9][j] = _mm_extract_epi8(xmm_result, 9);
        //   if (_mm_extract_epi8(vdata_index, 10) != -1) res[i][10][j] = _mm_extract_epi8(xmm_result, 10);
        //   if (_mm_extract_epi8(vdata_index, 11) != -1) res[i][11][j] = _mm_extract_epi8(xmm_result, 11);
        //   if (_mm_extract_epi8(vdata_index, 12) != -1) res[i][12][j] = _mm_extract_epi8(xmm_result, 12);
        //   if (_mm_extract_epi8(vdata_index, 13) != -1) res[i][13][j] = _mm_extract_epi8(xmm_result, 13);
        //   if (_mm_extract_epi8(vdata_index, 14) != -1) res[i][14][j] = _mm_extract_epi8(xmm_result, 14);
        //   if (_mm_extract_epi8(vdata_index, 15) != -1) res[i][15][j] = _mm_extract_epi8(xmm_result, 15);
        // }

        for (int l = 0; l < M; l++) if (vdata_index[l] != -1) {
          res[i][l][j] = ((char *)(&xmm_result))[l]; 
          // cout << (int)((char *)(&xmm_result))[l] << " ";
        }
        // cout << endl;
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}

void ShuffleArray() {
    // 创建一个混洗掩码
    __m128i vector = _mm_set_epi8(8, 7, 6, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -6, -7);
    __m128i shuffleMask = _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8,7, 6, 5, 4, 3, 2, 1, 0);

    // 使用混洗掩码对数组进行混洗
    __m128i xmm_result = _mm_shuffle_epi8(vector, shuffleMask);
    for (int i = 0; i < 16; i++)
        {
                printf("%.2d ", ((char *)(&xmm_result))[i]);
        }
        printf("\n");
}

int main() {
    // M为8或16或32
    int B = 1, K = 32, L = 1, M = 32;
    signed char ***src = create3D(B, K, L);
    uint8_t **index = create2D(B, M);
    init3D(src, B, K, L);
    init2D(index, B, M, K);
    signed char ***res1 = calIndexTime(src, index, B, K, L, M);
    signed char ***res2 = calIndexTimeByPSHUFL(src, index, B, K, L, M);
    isEqual(res1, res2, B, M, L);
    // print3D(src, B, K, L);
    // print2D(index, B, M);
    // print3D(res1, B, M, L);
    // print3D(res2, B, M, L);
    delete3D(src, B, K);
    delete2D(index, B);
    delete3D(res1, B, M);
    delete3D(res2, B, M);
    // ShuffleArray();
    return 0;
}