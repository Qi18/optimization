# optimization
优化算子的实现

## 编译ARM CPU指令
g++ CPU.cpp -o CPU

## 编译ARM GPU的指令
g++ GPU.cpp -o GPU.out -framework OpenCL

## 编译x86 CPU的指令
g++ -msse4.1 CPU_x86.cpp -o CPU

### GPU失效情况
+ localWorkSize太大
+ globalWorkSize不被localWorkSize整除
