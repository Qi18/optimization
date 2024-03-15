#include <vector>
#include <time.h>
#include <list>
#include <arm_neon.h>
#include "Matrix.h"

using namespace std;

signed char*** calIndexTime(signed char ***src, uint8_t **index, int B, int K, int L) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  clock_t t1,t2;
  float val = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, 8, L);
  for (int i = 0; i < B; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < L; k++) {
        if (index[i][j] < 0 || index[i][j] >= K) continue;
        t2 = clock();
        res[i][j][k] = src[i][index[i][j]][k];
        val += (float)(clock()-t2);
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU time: " << nanoseconds << " ns" << std::endl;
  return res;
}


signed char*** calIndexTimeByTBL(signed char ***src, uint8_t **index, int B, int K, int L) {
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, 8, L);
  for (int i = 0; i < B; i++) {
    for (int k = 0; k < K / 16; k++) {
      int8x16_t vdata;
      int8x8_t vdata_index;
      vdata_index = vld1_u8(index[i]);
      vdata_index = vreinterpret_u8_s8(vdata_index);
      // uint32x4_t cmp2 = vcltq_u32(val, vdupq_n_u32(10));
      for (int l = 0; l < 8; l++) vdata_index[l] = vdata_index[l] / 16 == k ? vdata_index[l] % 16 : -1;
      for (int j = 0; j < L; j++) {
        for (int l = 0; l < 16; l++) vdata[l] = src[i][k * 16 + l][j];
        // int8x8_t tbl_2= vtbl1_s8(vdata , vdata_index);
        int8x8_t tbl_2 = vqtbl1_s8(vdata, vdata_index);
        for (int l = 0; l < 8; l++) if (vdata_index[l] != -1) res[i][l][j] = tbl_2[l];
      }
      // vst1_s8(&(res[i][j][0]), tbl_2);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}



int main() {
  int B = 1000, K = 16, L = 384, M = 8;
  signed char ***src = create3D(B, K, L);
  uint8_t **index = create2D(B, M);
  init3D(src, B, K, L);
  init2D(index, B, M, K);
  signed char ***res2 = calIndexTime(src, index, B, K, L);
  signed char ***res1 = calIndexTimeByTBL(src, index, B, K, L);
  isEqual(res1, res2, B, M, L);
  // print3D(src, B, K, L);
  // print2D(index, B, M);
  // print3D(res2, B, M, L);
  // print3D(res1, B, M, L);
  delete3D(src, B, K);
  delete2D(index, B);
  delete3D(res2, B, M);
  delete3D(res1, B, M);
  // neon_tbl_example();
}