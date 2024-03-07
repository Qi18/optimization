
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


const int ARRAY_SIZE = 100000;

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
    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

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
                      int8_t ***src, uint8_t **index, int B, int K, int L)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int8_t) * B * K * L, src, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(uint8_t) * B * 8, index, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   sizeof(int8_t) * B * 8 * L, NULL, NULL);
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

void runGPU(signed char ***src, uint8_t **index, int B, int K, int L, int8_t ***res, const char* filename){
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = { 0, 0, 0 };
    cl_int errNum;
   // uint64_t t1,t2,t3;
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

    //创建内存对象
    if (!CreateMemObjects(context, memObjects, src, index, B, K, L))//a
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return ;
    }
    cout << "CreateMemObjects" << endl;
    // 五、 设置内核数据并执行内核
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

    size_t globalWorkSize[1] = { B };
    size_t localWorkSize[1] = { 1 };

    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);

    // 六、 读取执行结果并释放OpenCL资源
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
                                 0, ARRAY_SIZE * sizeof(float), res,
                                 0, NULL, NULL);

    t3 = clock();  //mach_absolute_time();
    printf("gpu %s t = %.8f\n", filename, (float)(t3-t1)/CLOCKS_PER_SEC);
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
    Cleanup(context, commandQueue, program, kernel, memObjects);
}


int main()
{
    int B = 1, K = 16, L = 1, M = 8;
    signed char ***src = create3D(B, K, L);
    uint8_t **index = create2D(B, M);
    init3D(src, B, K, L);
    init2D(index, B, M, K);
    signed char ***res1 = create3D(B, M, L);
    signed char ***res2 = create3D(B, M, L);
    const char* shuffle_filename = "./shuffle_optimization.cl";
    const char* index_filename = "./index.cl";
    // runGPU(src, index, B, K, L, res1, shuffle_filename);
    runGPU(src, index, B, K, L, res2, index_filename);
    // isEqual(res1, res2, B, M, L);
    // print3D(src, B, K, L);
    // print2D(index, B, M);
    // print3D(res2, B, M, L);
    print3D(res2, B, M, L);
    delete3D(src, B, K);
    delete2D(index, B);
    delete3D(res2, B, M);
    delete3D(res1, B, M);
    return 0;
}
