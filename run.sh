## 编译arm CPU的指令
g++ CPU_arm.cpp -std=c++11 -o output/CPU_arm.out && ./output/CPU_arm.out | tee ./output/CPU_M1.txt

## 编译arm GPU的指令
## g++ GPU_arm.cpp -o output/GPU_arm.out -framework OpenCL && ./output/GPU_arm.out | tee ./output/GPU-M1.txt

## g++ test_gpu_mm.cpp -o output/test_gpu_mm.out -framework OpenCL && ./output/test_gpu_mm.out | tee ./output/test_gpu_mm-M1.txt
