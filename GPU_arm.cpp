
//  main.cpp
//  OpenCL
//
//  Created by xxx on 2017/9/19.
//  Copyright © 2017年 xxx. All rights reserved.
//


#include <OpenCL/OpenCL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include <mach/mach_time.h>
#include "Matrix.h"

//一、 选择OpenCL平台并创建一个上下文
cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;

    //选择可用的平台中的第一个
    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
    if (errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        std::cerr << "Failed to find any OpenCL platforms." << std::endl;
        return NULL;
    }

    //创建一个OpenCL上下文环境
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)firstPlatformId,
        0
    };
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
                                      NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS)
    {
        std::cout << "Could not create GPU context, trying CPU..." << std::endl;
        context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU,
                                          NULL, NULL, &errNum);
        if (errNum != CL_SUCCESS)
        {
            std::cerr << "Failed to create an OpenCL GPU or CPU context." << std::endl;
            return NULL;
        }
    }
    return context;
}


//二、 创建设备并创建命令队列
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_int errNum;
    cl_device_id *devices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;

    // 获取设备缓冲区大小
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);

    if (deviceBufferSize <= 0)
    {
        std::cerr << "No devices available.";
        return NULL;
    }

    // 为设备分配缓存空间
    devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);

    //选取可用设备中的第一个
    commandQueue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE, NULL);

    *device = devices[0];
    delete[] devices;
    return commandQueue;
}


// 三、创建和构建程序对象
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int errNum;
    cl_program program;

    std::ifstream kernelFile(fileName, std::ios::in);
    if (!kernelFile.is_open())
    {
        std::cerr << "Failed to open file for reading: " << fileName << std::endl;
        return NULL;
    }

    std::ostringstream oss;
    oss << kernelFile.rdbuf();

    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&srcStr,
                                        NULL, NULL);

    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        // 输出编译错误信息
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);
 
        std::cerr << "Error in kernel: " << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);//释放程序对象空间
        return NULL;
    }

    return program;
}

//创建和构建程序对象
bool CreateMemObjects(cl_context context, cl_mem memObjects[3],
                      int8_t *src, uint8_t *index, int B, int K, int L, int M)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int8_t) * B * K * L, src, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(uint8_t) * B * M, index, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   sizeof(int8_t) * B * M * L, NULL, NULL); 
    return true;
}


// 释放OpenCL资源
void Cleanup(cl_context context, cl_command_queue commandQueue,
             cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
    for (int i = 0; i < 3; i++)
    {
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

size_t runGPU(int8_t *src, uint8_t *index, int B, int L, int K, int M,
              int8_t *res, const char *filename) {
    // src:[B, L, K] + index:[B, M] -> [B, L, K]
  cl_context context = 0;
  cl_command_queue commandQueue = 0;
  cl_program program = 0;
  cl_device_id device = 0;
  cl_kernel kernel = 0;
  cl_mem memObjects[3] = {0, 0, 0};
  cl_int errNum;
  clock_t t1, t2, t3;
  t1 = clock(); // mach_absolute_time();

  // 一、选择OpenCL平台并创建一个上下文
  context = CreateContext();

  // 二、 创建设备并创建命令队列
  commandQueue = CreateCommandQueue(context, &device);

  // 创建和构建程序对象
  program = CreateProgram(context, device, filename); //"HelloWorld.cl");

  // 四、 创建OpenCL内核并分配内存空间
  kernel = clCreateKernel(program, "hello_kernel", NULL);

  // //创建要处理的数据
  // float result[ARRAY_SIZE];
  // float a[ARRAY_SIZE];
  // float b[ARRAY_SIZE];
  // for (int i = 0; i < ARRAY_SIZE; i++)
  // {
  //     a[i] = (float)i;
  //     b[i] = (float)(ARRAY_SIZE - i);
  // }

  // t1 = clock();  //mach_absolute_time();
  // printf("t1 = %.8f\n",(double)t1);
  // for(int j = 0;j <  ARRAY_SIZE;j++){
  //     result[j] = a[j]+b[j];

  // }

  // t2 = clock(); //mach_absolute_time();
  // printf("t2 = %.8f\n",(double)t2);

  // 创建内存对象
  if (!CreateMemObjects(context, memObjects, src, index, B, K, L, M)) // a
  {
    Cleanup(context, commandQueue, program, kernel, memObjects);
    return -1;
  }
  // 五、 设置内核数据并执行内核
  errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
  errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
  errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

  errNum |= clSetKernelArg(kernel, 3, sizeof(int), (void *)&B);
  errNum |= clSetKernelArg(kernel, 4, sizeof(int), (void *)&L);
  errNum |= clSetKernelArg(kernel, 5, sizeof(int), (void *)&M);
  errNum |= clSetKernelArg(kernel, 6, sizeof(int), (void *)&K);

  size_t globalWorkSize[2] = {(size_t)B, (size_t)L};
  size_t localWorkSize[2] = {1, 1};

  cl_event ev;
  errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize,
                                  localWorkSize, 0, NULL, &ev);

  // 六、 读取执行结果并释放OpenCL资源
  errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0,
                               B * L * M * sizeof(int8_t), res, 1, &ev, NULL);

  // if (errNum != CL_SUCCESS)
  // {
  //     fprintf(stderr, "failed to read buffer\n");
  //     return ;
  // }
  // errNum |= clFinish(commandQueue);
  errNum |= clWaitForEvents(1, &ev);
  // errNum |= clFinish(commandQueue);

  cl_ulong time_start;
  cl_ulong time_end;
  clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(time_start),
                          &time_start, NULL);
  clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(time_end),
                          &time_end, NULL);

  double nanoSeconds = time_end - time_start;
  std::cout << filename << std::endl;
  std::cout << std::fixed << std::setprecision(0) << time_start << std::endl
            << time_end << std::endl
            << nanoSeconds << std::endl
            << std::endl;
  // std::cout << "Executed program succesfully." << std::endl;
  Cleanup(context, commandQueue, program, kernel, memObjects);
  return nanoSeconds;
}

int main()
{
    int B = 20, K = 16, L = 100, M = 16;
    int8_t *src = createOne<int8_t>(B * L * K);
    uint8_t *index = createOne<uint8_t>(B * M);
    initOne<int8_t>(src, B * L * K);
    initOne<uint8_t>(index, B * M, K);
    int8_t *res1 = createOne<int8_t>(B * L * M);
    int8_t *res2 = createOne<int8_t>(B * L * M);
    const char* shuffle_filename = "./shuffle_optimization.cl";
    const char* index_filename = "./index.cl";
    runGPU(src, index, B, L, K, M, res2, index_filename);
    runGPU(src, index, B, L, K, M, res1, shuffle_filename);
    isEqual(res1, res2, B * M * L);
    // printOne(src, B, L, K);
    // printOne(index, B, M);
    // printOne(res1, B, L, M);
    // printOne(res2, B, L, M);
    deleteOne(src);
    deleteOne(index);
    deleteOne(res2);
    deleteOne(res1);
    return 0;
}
