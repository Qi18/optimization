# optimization
优化算子的实现

## 编译CPU指令
g++ CPU.cpp -o CPU

## 编译GPU的指令
g++ GPU.cpp -o GPU.out -framework OpenCL

### GPU失效情况
+ localWorkSize太大
+ globalWorkSize不被localWorkSize整除
