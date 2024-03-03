#!/bin/bash
 
# 运行程序，so库，模型，图片等文件路径
DATA_DIR=/Users/kaka/Desktop/ceshi/
# 复制到Android系统的路径
ANDROID_DIR=/data/local/tmp
 
function runInAndroidEnv(){
	
	#将准备好的文件push到Android
	adb push $DATA_DIR $ANDROID_DIR
 
	adb shell chmod 777 $ANDROID_DIR/ceshi/sheet_phone_test
  
    #LD_LIBRARY_PATH是Linux环境变量名，该环境变量主要用于指定查找共享库（动态链接库）时除了默认路径之外的其他路径
    #执行测试程序
    adb shell "LD_LIBRARY_PATH=$ANDROID_DIR/ceshi/libs ${ANDROID_DIR}/ceshi/sheet_phone_test ${ANDROID_DIR}/ceshi/pictures/ ${ANDROID_DIR}/ceshi/models_sheet_phone 4 1080 1920 > $ANDROID_DIR/ceshi/log.txt"
    
    #导出运行日志
    adb pull $ANDROID_DIR/ceshi/log.txt $DATA_DIR
    
 
}
 
 
runInAndroidEnv