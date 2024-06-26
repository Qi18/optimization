#include <random>
#include <iostream>
#include <chrono>

using namespace std;

bool isEqual(signed char ***arr1, signed char ***arr2, int K, int M, int L) {
  for (int i = 0; i < K; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < L; k++) {
        if (arr1[i][j][k] != arr2[i][j][k]) {
          cout << i << " " << j << " " << k << endl;
          cout << (int)arr1[i][j][k] << " " << (int)arr2[i][j][k] << endl;
          cout << "not equal" << endl;
          return false;
        }
      }
    }
  }
  cout << "equal" << endl;
  return true;
}

signed char*** create3D(int B, int K, int L) {
  signed char ***arr = new signed char**[B];
  for (int i = 0; i < B; ++i) {
      arr[i] = new signed char*[K];
      for (int j = 0; j < K; ++j) {
          arr[i][j] = new signed char[L];
      }
  }
  return arr;
}

uint8_t** create2D(int B, int M) {
  uint8_t **arr = new uint8_t*[B];
  for (int i = 0; i < B; ++i) {
      arr[i] = new uint8_t[M];
  }
  return arr;
}

void delete3D(signed char ***arr, int B, int K) {
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < K; ++j) {
          delete[] arr[i][j];
      }
      delete[] arr[i];
  }
  delete[] arr;
}

void delete2D(uint8_t **arr, int B) {
  for (int i = 0; i < B; ++i) {
      delete[] arr[i];
  }
  delete[] arr;
}

void print3D(signed char ***arr, int B, int K, int L) {
  cout << "print 3D" << endl;
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < K; ++j) {
          for (int k = 0; k < L; ++k) {
              cout << (int)arr[i][j][k] << " ";
          }
          cout << endl;
      }
      cout << endl << endl;
  }
  cout << endl;
}

void print2D(uint8_t **arr, int B, int M) {
  cout << "print 2D" << endl;
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < M; ++j) {
          cout << (int)arr[i][j] << " ";
      }
      cout << endl;
  }
  cout << endl;
}

void init3D(signed char ***arr, int B, int K, int L) {
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

void init2D(uint8_t **arr, int B, int M, int K) {
  for (int i = 0; i < B; ++i) {
      for (int j = 0; j < M; ++j) {
          arr[i][j] = (i * M + j) % K;
      }
  }
}

template <typename T>
T* createOne(int length) {
  T *arr = new T[length];
  return arr;
}

template <typename T>
void initOne(T* arr, int length) {
  std::random_device rd;
  // 使用随机设备生成引擎
  std::mt19937 gen(rd());
  // 定义随机数分布
  std::uniform_int_distribution<> dis(1, 100); // 生成1到100之间的随机整数
  for (int i = 0; i < length; ++i) {
    arr[i] = dis(gen);
  }
}

template <typename T>
void initOne(T* arr, int length, int range) {
  for (int i = 0; i < length; ++i) {
    arr[i] = i % range;
  }
}

template <typename T>
void printOne(T* arr, int B, int M) {
  cout << "print 2D" << endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < M; ++j) {
      cout << (int)arr[i * M + j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}

template <typename T>
void printOne(T* arr, int B, int M, int L) {
  cout << "print 3D" << endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < M; ++j) {
      for (int p = 0; p < L; ++p) {
        cout << (int)arr[i * M * L + j * L + p] << " ";
      }
      cout << endl;
    }
    cout << endl << endl;
  }
  cout << endl;
}

template <typename T>
void deleteOne(T* arr) {
  delete[] arr;
}

template <typename T> 
bool isEqual(T *arr1, T *arr2, int length) {
  for (int i = 0; i < length; i++) {
    if (arr1[i] != arr2[i]) {
      cout << "not equal" << endl;
      return false;
    }
  }
  cout << "equal" << endl;
  return true;
}