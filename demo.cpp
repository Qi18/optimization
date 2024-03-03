#include <iostream>
#include <vector>
#include <time.h>
#include <list>
#include <arm_neon.h>
#include <random>

using namespace std;
char*** create3D(int B, int K, int L);
char** create2D(int B, int M);
void print3D(char ***arr, int B, int K, int L);


char*** calIndexTime(char ***src, char **index, int B, int K, int L) {
  // src:[B, K, L] + index:[B, M] -> [B, M, L]
  clock_t t1,t2;
  t1 = clock();
  char ***res = create3D(B, 8, L);
  for (int i = 0; i < B; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < L; k++) {
        if (index[i][j] < 0 || index[i][j] >= K) continue;
        res[i][j][k] = src[i][index[i][j]][k];
      }
    }
  }
  t2 = clock();
  printf("cpu t = %.8f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
  return res;
}


char*** calIndexTimeByTBL(char ***src, char **index, int B, int K, int L) {
  clock_t t1,t2;
  t1 = clock();
  char ***res = create3D(B, 8, L);
  for (int i = 0; i < B; i++) {
    // int8x8_t vdata_index = vld1_dup_s8(index[i]);
    for (int j = 0; j < L; j++) {
      for (int k = 0; k < K / 8; k++) {
        // int8x8_t vdata = vld1_dup_s8(src[i][l] + k * 8 * 8);
        int8x8_t vdata, vdata_index;
        for (int l = 0; l < 8; l++) {
          vdata[l] = src[i][k * 8 + l][j];
          vdata_index[l] = index[i][l] / 8 == k ? index[i][l] % 8 : -1;
          // cout << (int)vdata[l] << " ";
          // cout << (int)vdata_index[l] << endl;
        }
        int8x8_t tbl_2= vtbl1_s8(vdata , vdata_index);
        for (int l = 0; l < 8; l++) {
          if (vdata_index[l] != -1) res[i][l][j] = tbl_2[l];
        }
      }
      // vst1_s8(&(res[i][j][0]), tbl_2);
    }
  }
  t2 = clock();
  printf("cpu tbl t = %.8f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
  return res;
}

bool isEqual(char ***arr1, char ***arr2, int K, int M, int L) {
  for (int i = 0; i < K; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < L; k++) {
        if (arr1[i][j][k] != arr2[i][j][k]) {
          cout << "not equal" << endl;
          return false;
        }
      }
    }
  }
  cout << "equal" << endl;
  return true;
}

void neon_tbl_example()
{
  int8x8_t vdata = {0,1,2,3,4,5,6,7};
  int8x8_t index = {1,1,2,3,3,7,-1,9};
  int8x8_t tbl_2= vtbl1_s8(vdata , index);
  for (int i = 0; i < 8; i++) {
    cout << (int)tbl_2[i] << " ";
  }
}

char*** create3D(int B, int K, int L) {
  char ***arr = new char**[B];
  for (int i = 0; i < B; ++i) {
      arr[i] = new char*[K];
      for (int j = 0; j < K; ++j) {
          arr[i][j] = new char[L];
      }
  }
  return arr;
}

char** create2D(int B, int M) {
  char **arr = new char*[B];
  for (int i = 0; i < B; ++i) {
      arr[i] = new char[M];
  }
  return arr;
}

void delete3D(char ***arr, int B, int K) {
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < K; ++j) {
          delete[] arr[i][j];
      }
      delete[] arr[i];
  }
  delete[] arr;
}

void delete2D(char **arr, int B) {
  for (int i = 0; i < B; ++i) {
      delete[] arr[i];
  }
  delete[] arr;
}

void print3D(char ***arr, int B, int K, int L) {
  cout << "print 3D" << endl;
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < K; ++j) {
          for (int k = 0; k < L; ++k) {
              cout << arr[i][j][k] << " ";
          }
          cout << endl;
      }
      cout << endl;
  }
  cout << endl;
}

void print2D(char **arr, int B, int M) {
  cout << "print 2D" << endl;
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < M; ++j) {
          cout << arr[i][j] << " ";
      }
      cout << endl;
  }
  cout << endl;
}

void init3D(char ***arr, int B, int K, int L) {
  // 使用随机设备生成种子
  std::random_device rd;
  // 使用随机设备生成引擎
  std::mt19937 gen(rd());
  // 定义随机数分布
  std::uniform_int_distribution<> dis(1, 100); // 生成1到100之间的随机整数
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < K; ++j) {
          for (int k = 0; k < L; ++k) {
              arr[i][j][k] = dis(gen);
          }
      }
  }
}

void init2D(char **arr, int B, int M, int K) {
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < M; ++j) {
          arr[i][j] = (i * M + j) % K;
      }
  }
}

int main() {
  int B = 100, K = 1024, L = 1024, M = 8;
  char ***src = create3D(B, K, L);
  char **index = create2D(B, M);
  init3D(src, B, K, L);
  init2D(index, B, M, K);
  char ***res2 = calIndexTime(src, index, B, K, L);
  char ***res1 = calIndexTimeByTBL(src, index, B, K, L);
  isEqual(res1, res2, B, M, L);
  // print3D(src, B, K, L);
  // print2D(index, B, M);
  // print3D(res2, B, M, L);
  // print3D(res1, B, M, L);
  delete3D(src, B, K);
  delete2D(index, B);
  delete3D(res2, B, M);
  delete3D(res1, B, M);
}