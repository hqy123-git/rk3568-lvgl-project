cmake_minimum_required(VERSION 3.10)

# 设置交叉编译
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
#下面两行需要根据实际情况写自己虚拟机交叉编译器的路径
set(CMAKE_C_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/arm-linux-g++)

project(lvgl_project)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 关键修改：创建自定义命令将lv_conf.h复制到lvgl目录
configure_file(
    ${PROJECT_SOURCE_DIR}/my_src/lv_conf.h
    ${PROJECT_SOURCE_DIR}/lvgl/lv_conf.h
    COPYONLY
)

# 添加头文件包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lvgl
    ${PROJECT_SOURCE_DIR}/my_src
    
)

# 添加子目录
add_subdirectory(lvgl)
add_subdirectory(my_src)