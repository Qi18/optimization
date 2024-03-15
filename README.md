# optimization
优化算子的实现

## 编译ARM CPU指令
g++ CPU_arm.cpp -o CPU

## 编译ARM GPU的指令
g++ GPU_arm.cpp -o GPU -framework OpenCL

## 编译x86 CPU的指令
g++ -msse4.1 CPU_x86.cpp -o CPU // 使用128bit的sse指令
g++ -avx2 CPU_x86.cpp -o CPU // 使用256bit的avx2指令


### GPU失效情况
+ localWorkSize太大
+ globalWorkSize不被localWorkSize整除
