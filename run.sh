g++ CPU.cpp -o CPU.out && ./CPU.out | tee CPU-M1.txt

## 编译GPU的指令
g++ GPU.cpp -o GPU.out -framework OpenCL && ./GPU.out | tee GPU-M1.txt

g++ test_gpu_mm.cpp -o test_gpu_mm.out -framework OpenCL && ./test_gpu_mm.out | tee test_gpu_mm-M1.txt