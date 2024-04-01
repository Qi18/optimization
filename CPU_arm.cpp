#include <vector>
#include <time.h>
#include <list>
#include <arm_neon.h>
#include "Matrix.h"

using namespace std;

signed char*** calIndexTime(signed char ***src, uint8_t **index, int B, int K, int L, int M = 16) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  clock_t t1,t2;
  float val = 0.0;
  auto start = std::chrono::high_resolution_clock::now();
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
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU time: " << nanoseconds << " ns" << std::endl;
  return res;
}


signed char*** calIndexTimeByTBL(signed char ***src, uint8_t **index, int B, int K, int L, int M = 16) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  auto start = std::chrono::high_resolution_clock::now();
  signed char ***res = create3D(B, M, L);
  for (int i = 0; i < B; i++) {
    //一个index对应不同区域的多个mask的提前计算
    int8x16_t* vdata_index_masks = new int8x16_t[K / M];
    int8x16_t vdata_index = vreinterpretq_s8_u8(vld1q_u8(index[i]));
    int8x16_t vdata_index_remain;
    // vdata_index = vreinterpret_u8_s8(vdata_index);
    for (int l = 0; l < M; l++) vdata_index_remain[l] = vdata_index[l] % M;
    for (int k = 0; k < K / M; k++) {
      for (int l = 0; l < M; l++) vdata_index_masks[k][l] = vdata_index[l] / M == k ? -1 : 0;//之后会做逻辑运算
    }
      // vdata_indexs[k][l] = vdata_index[l] / 16 == k ? vdata_index[l] % 16 : -1;
    for (int j = 0; j < L; j++) {
      int8x16_t tbl_res;
      for (int l = 0; l < M; l++) tbl_res[l] = 0;
      for (int k = 0; k < K / M; k++) {
        int8x16_t vdata;
        for (int l = 0; l < M; l++) vdata[l] = src[i][k * 16 + l][j];//这里不知道转置能不能优化
        int8x16_t tbl_temp_res = vqtbl1q_s8(vdata, vdata_index_remain);
        //and操作，过滤无效的向量元素
        int8x16_t tbl_temp_res_masked = vandq_s8(tbl_temp_res, vdata_index_masks[k]);
        //or操作，合并成一个结果向量
        tbl_res = vorrq_s8(tbl_res, tbl_temp_res_masked);
      }
      // for (int l = 0; l < 8; l++) if (vdata_index[l] != -1) res[i][l][j] = tbl_2[l];
      for (int l = 0; l < M; l++) res[i][l][j] = tbl_res[l];
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU tbl time: " << nanoseconds << " ns" << std::endl;
  return res;
}



int main() {
  //测试下来，K<130 * 16时，TBL的效率更高
  int B = 5, K = 16 * 130, L = 100000, M = 16;
  signed char ***src = create3D(B, K, L);
  uint8_t **index = create2D(B, M);
  init3D(src, B, K, L);
  init2D(index, B, M, K);
  signed char ***res1 = calIndexTime(src, index, B, K, L);
  signed char ***res2 = calIndexTimeByTBL(src, index, B, K, L);
  isEqual(res1, res2, B, M, L);
  // print3D(src, B, K, L);
  // print2D(index, B, M);
  // print3D(res1, B, M, L);
  // print3D(res2, B, M, L);
  delete3D(src, B, K);
  delete2D(index, B);
  delete3D(res1, B, M);
  delete3D(res2, B, M);
  // neon_tbl_example();
}