#include <OpenCL/OpenCL.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <random>
#include <fstream>
#include <arm_neon.h>

bool isEqual(signed char ***arr1, signed char ***arr2, int K, int M, int L) {
  for (int i = 0; i < K; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < L; k++) {
        if (arr1[i][j][k] != arr2[i][j][k]) {
          std::cout << "not equal" << std::endl;
          return false;
        }
      }
    }
  }
  std::cout << "equal" << std::endl;
  return true;
}

signed char ***create3D(int B, int K, int L) {
  signed char ***arr = new signed char **[B];
  for (int i = 0; i < B; ++i) {
    arr[i] = new signed char *[K];
    for (int j = 0; j < K; ++j) {
      arr[i][j] = new signed char[L];
    }
  }
  return arr;
}

uint8_t **create2D(int B, int M) {
  uint8_t **arr = new uint8_t *[B];
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
  std::cout << "print 3D" << std::endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < K; ++j) {
      for (int k = 0; k < L; ++k) {
        std::cout << (int)arr[i][j][k] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
  }
  std::cout << std::endl;
}

template <typename T>
void print2D(T **arr, int B, int M) {
  std::cout << "print 2D" << std::endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < M; ++j) {
      std::cout << (int)arr[i][j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
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

template <typename T> T *createOne(int length) {
  T *arr = new T[length];
  return arr;
}

template <typename T> void initOne(T *arr, int length) {
  // std::random_device rd;
  // // 使用随机设备生成引擎
  // std::mt19937 gen(rd());
  // // 定义随机数分布
  // std::uniform_int_distribution<> dis(1, 100); // 生成1到100之间的随机整数
  for (int i = 0; i < length; ++i) {
    // arr[i] = dis(gen);
    arr[i] = 1;
  }
}

template <typename T> void initOneZero(T *arr, int length) {
  // std::random_device rd;
  // // 使用随机设备生成引擎
  // std::mt19937 gen(rd());
  // // 定义随机数分布
  // std::uniform_int_distribution<> dis(1, 100); // 生成1到100之间的随机整数
  for (int i = 0; i < length; ++i) {
    // arr[i] = dis(gen);
    arr[i] = 0;
  }
}

template <typename T> void initOne(T *arr, int length, int range) {
  for (int i = 0; i < length; ++i) {
    arr[i] = i % range;
  }
}

template <typename T> void printOne(T *arr, int B, int M) {
  std::cout << "print 2D" << std::endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < M; ++j) {
      std::cout << (int)arr[i * M + j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

template <typename T> void printOne(T *arr, int B, int M, int L) {
  std::cout << "print 3D" << std::endl;
  for (int i = 0; i < B; ++i) {
    for (int j = 0; j < M; ++j) {
      for (int p = 0; p < L; ++p) {
        std::cout << (int)arr[i * M * L + j * L + p] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
  }
  std::cout << std::endl;
}

template <typename T> void deleteOne(T *arr) { delete[] arr; }

template <typename T> bool isEqual(T *arr1, T *arr2, int length) {
  for (int i = 0; i < length; i++) {
    if (arr1[i] != arr2[i]) {
      std::cout << "not equal" << std::endl;
      return false;
    }
  }
  std::cout << "equal" << std::endl;
  return true;
}

cl_context CreateContext() {
  cl_int errNum;
  cl_uint numPlatforms;
  cl_platform_id firstPlatformId;
  cl_context context = NULL;

  // 选择可用的平台中的第一个
  errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
  if (errNum != CL_SUCCESS || numPlatforms <= 0) {
    std::cerr << "Failed to find any OpenCL platforms." << std::endl;
    return NULL;
  }

  // 创建一个OpenCL上下文环境
  cl_context_properties contextProperties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)firstPlatformId, 0};
  context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL,
                                    NULL, &errNum);
  if (errNum != CL_SUCCESS) {
    std::cout << "Could not create GPU context, trying CPU..." << std::endl;
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU,
                                      NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS) {
      std::cerr << "Failed to create an OpenCL GPU or CPU context."
                << std::endl;
      return NULL;
    }
  }
  return context;
}

// 二、 创建设备并创建命令队列
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device) {
  cl_int errNum;
  cl_device_id *devices;
  cl_command_queue commandQueue = NULL;
  size_t deviceBufferSize = -1;

  // 获取设备缓冲区大小
  errNum =
      clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);

  if (deviceBufferSize <= 0) {
    std::cerr << "No devices available.";
    return NULL;
  }

  // 为设备分配缓存空间
  devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
  errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize,
                            devices, NULL);

  // 选取可用设备中的第一个
  commandQueue = clCreateCommandQueue(context, devices[0],
                                      CL_QUEUE_PROFILING_ENABLE, NULL);

  *device = devices[0];
  delete[] devices;
  return commandQueue;
}

// 三、创建和构建程序对象
cl_program CreateProgram(cl_context context, cl_device_id device,
                         const char *fileName) {
  cl_int errNum;
  cl_program program;

  std::ifstream kernelFile(fileName, std::ios::in);
  if (!kernelFile.is_open()) {
    std::cerr << "Failed to open file for reading: " << fileName << std::endl;
    return NULL;
  }

  std::ostringstream oss;
  oss << kernelFile.rdbuf();

  std::string srcStdStr = oss.str();
  const char *srcStr = srcStdStr.c_str();
  program =
      clCreateProgramWithSource(context, 1, (const char **)&srcStr, NULL, NULL);

  errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (errNum != CL_SUCCESS) {
    // 输出编译错误信息
    char buildLog[16384];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                          sizeof(buildLog), buildLog, NULL);

    std::cerr << "Error in kernel: " << std::endl;
    std::cerr << buildLog;
    clReleaseProgram(program); // 释放程序对象空间
    return NULL;
  }

  return program;
}

// 创建和构建程序对象
bool CreateMemObjects(cl_context context, cl_mem memObjects[3], int8_t *A,
                      int8_t *B, int N, int K, int M) {
  memObjects[0] =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(int8_t) * N * K, A, NULL);
  memObjects[1] =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(int8_t) * K * M, B, NULL);
  memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                 sizeof(int8_t) * N * M, NULL, NULL);
  return true;
}

// 释放OpenCL资源
void Cleanup(cl_context context, cl_command_queue commandQueue,
             cl_program program, cl_kernel kernel, cl_mem memObjects[3]) {
  for (int i = 0; i < 3; i++) {
    if (memObjects[i] != 0)
      clReleaseMemObject(memObjects[i]);
  }
  if (commandQueue != 0)
    clReleaseCommandQueue(commandQueue);

  if (kernel != 0)
    clReleaseKernel(kernel);

  if (program != 0)
    clReleaseProgram(program);

  if (context != 0)
    clReleaseContext(context);
}

size_t runGPU(int8_t *A, int8_t *B,int N, int K, int M, int8_t *res, const char* filename){
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = { 0, 0, 0 };
    cl_int errNum;
    clock_t t1,t2,t3;
    t1 = clock();  //mach_absolute_time();

    // 一、选择OpenCL平台并创建一个上下文
    context = CreateContext();

    // 二、 创建设备并创建命令队列
    commandQueue = CreateCommandQueue(context, &device);

    //创建和构建程序对象
    program = CreateProgram(context, device, filename);//"HelloWorld.cl");

    // 四、 创建OpenCL内核并分配内存空间
    kernel = clCreateKernel(program, "hello_kernel", NULL);

    //创建内存对象
    if (!CreateMemObjects(context, memObjects, A, B, N, K, M))//a
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }
    // 五、 设置内核数据并执行内核
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

    errNum |= clSetKernelArg(kernel, 3, sizeof(int), (void *)&N);
    errNum |= clSetKernelArg(kernel, 4, sizeof(int), (void *)&K);
    errNum |= clSetKernelArg(kernel, 5, sizeof(int), (void *)&M);

    size_t globalWorkSize[2] = {(size_t)N, (size_t)M};
    size_t localWorkSize[2] = {1, 1};

    cl_event ev;
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, &ev);

    // 六、 读取执行结果并释放OpenCL资源
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
                                 0, N * M * sizeof(int8_t), res,
                                 1, &ev, NULL);

    // if (errNum != CL_SUCCESS)
    // {
    //     fprintf(stderr, "failed to read buffer\n");
    //     return ;
    // }
    // errNum |= clFinish(commandQueue);
    errNum |= clWaitForEvents(1, &ev);
    errNum |= clFinish(commandQueue);

    cl_ulong time_start;
    cl_ulong time_end;
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    double nanoSeconds = time_end - time_start;
    std::cout << filename << std::endl;
    std::cout << std::fixed << std::setprecision(0) << time_start << std::endl 
              << time_end << std::endl
              << nanoSeconds << std::endl << std::endl;
    // std::cout << "Executed program succesfully." << std::endl;
    Cleanup(context, commandQueue, program, kernel, memObjects);
    return nanoSeconds;
}

class TestMatMulGPU {
public:
  size_t run(int M, int N, int K) {
    int8_t *A = createOne<int8_t>(N * K);
    int8_t *B = createOne<int8_t>(K * M);
    initOne<int8_t>(A, N * K); 
    initOne<int8_t>(B, K * M);
    int8_t *res = createOne<int8_t>(N * M);
    const char* kernel_filename = "./test_matmul.cl";
    size_t time_kernel = runGPU(A, B, N, K, M, res, kernel_filename);
    return time_kernel;
  }
};

size_t runCPU(int M, int N, int K) {
  int8_t *A = createOne<int8_t>(N * K);
  int8_t *B = createOne<int8_t>(K * M);
  int8_t *res = createOne<int8_t>(N * M);
  initOne<int8_t>(A, N * K);
  initOne<int8_t>(B, K * M);
  // printOne(A, N, K);
  // printOne(B, K, M);
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      int sum = 0;
      for (int k = 0; k < K; k++) {
        sum += A[i * K + k] * B[k * M + j];
      }
      res[i * M + j] = sum;
    }
  }

  // printOne(res, N, M);    
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU time: " << nanoseconds << " ns" << std::endl;
  deleteOne(A);
  deleteOne(B);
  deleteOne(res);
  return nanoseconds;
}

size_t runCPUBlock(int M, int N, int K) {
  int8_t *A = createOne<int8_t>(N * K);
  int8_t *B = createOne<int8_t>(K * M);
  int8_t *res = createOne<int8_t>(N * M);
  initOneZero<int8_t>(res, N * M);
  initOne<int8_t>(A, N * K);
  initOne<int8_t>(B, K * M);
  // printOne(A, N, K);
  // printOne(B, K, M);
  auto start = std::chrono::high_resolution_clock::now();
  int row, col;
  for (row = 0; row < N; row++){  
    for (col = 0; col < ((M) & (~7)); col+=8){
      int16x8_t buf = vmovl_s8(vld1_s8(&res[row * M + col]));
      for (int k = 0; k < K; k++){
        int16x8_t va = vmovl_s8(vld1_s8(&B[k * M + col]));
        register int16_t vb = (int16_t) A[row * K + k];
        // for (int i = 0; i < 8; i++){
        //   std::cout << "va "<< (int)(va)[i] << " vb " << vb << " buf" << buf[i]<< std::endl;  
        // }
        buf = vmlaq_n_s16(buf, va, vb);
      }
      vst1_s8(&res[row * M + col], vqmovn_s16(buf)); 
      // for (int i = 0; i < 8; i++){
      //   std::cout << (int)(buf)[i] << " " <<(int)(vqmovn_s16(buf))[i] << std::endl;
      // }
    }
    for (; col < M; col++){//deal with boundaries
      register float temp = res[row * M + col];
      for (int k = 0; k < K; k++){
        temp += A[row * K + k] * B[k * M + col];
      }
      res[row * M + col] = temp;
    }
  }
  // printOne(res, N, M);                     
  auto end = std::chrono::high_resolution_clock::now();
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "CPU block time: " << nanoseconds << " ns" << std::endl;
  deleteOne(A);
  deleteOne(B);
  deleteOne(res);
  return nanoseconds;
}

int main() {
    TestMatMulGPU test;
    // int N = 100;
    
    // for (int M = 224; M <= 224 * 4; M += 224) {
    //   int K = M * 9;
    //   for (int B = 0; B < 10; B++) {
    //     auto gpu_time = test.run(M, N, K);
    //     auto cpu_time = runCPU(M, N, K);
    //     std::cout << "GPU/CPU [" << M << ", " << N << ", " << K << "]: " << std::fixed << std::setprecision(10) << (double)gpu_time / cpu_time << std::endl << std::endl;
    //   }
    // }
    int N = 16, K = 16, M = 16;
    // auto time = runCPUBlock(N, K, M);
    // auto time_kernel = runCPU(N, K, M);
    int8_t *A = createOne<int8_t>(N * K);
    int8_t *B = createOne<int8_t>(K * M);
    initOne<int8_t>(A, N * K); 
    initOne<int8_t>(B, K * M);
    int8_t *res = createOne<int8_t>(N * M);
    const char* kernel_filename = "./test_matmulblock.cl";
    size_t time_kernel = runGPU(A, B, N, K, M, res, kernel_filename);
    int8_t *res2 = createOne<int8_t>(N * M);
    const char* kernel_filename2 = "./test_matmul.cl";
    time_kernel = runGPU(A, B, N, K, M, res2, kernel_filename2);
    isEqual(res, res2, N * M * K);
    return 0;
}