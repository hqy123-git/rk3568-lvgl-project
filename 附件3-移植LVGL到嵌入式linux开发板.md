


# 1. 源代码下载

可以通过git下载
```sh
名称	仓库地址	描述
lvgl	https://github.com/lvgl/lvgl.git	包含了LVGL图形界面控件源码、驱动接口源代码以及例程
lv_port_linux_frame_buffer	https://github.com/lvgl/lv_port_linux_frame_buffer.git	适配有frame buffer的linux系统的接口
```

也可以直接拷贝我们下载好的源码包:
```sh
lvgl-release-v9.0.zip //包含了LVGL图形界面控件源码、驱动接口源代码以及例程
lvgl_port_linux-release-v9.0.zip //适配有frame buffer的linux系统的接口
```

注意：LVGL不同版本之间函数有较大区别

# 2. 步骤

## 2.1  准备好工程目录并把需要的文件拷贝其中
```sh
# 1.  创建一个工程目录(这个目录下会包括lvgl的代码和你自己的工程代码)
如:   创建一个名为 project_lvgl_xxx/ 及子目录    my_src

    project_lvgl_xxx/
        my_src/      <-----放你自己的源代码

       


# 2. 解压 lvgl-release-v9.0.zip  ，
#   然后把解压后的目录  lvgl-release-v9.0拷贝到第1步的  project_lvgl_xxx/下并改名 lvgl
    project_lvgl_xxx/
        lvgl/       <-------     
        my_src/

# 3. 解压 lvgl_port_linux-release-v9.0.zip 
#   然后把解压后的目录 lvgl_port_linux-release-v9.0目录的如下3个文件，拷贝到 project_lvgl_xxx/my_src子目录下
    lv_conf.h   main.c    mouse_cursor_icon.c 

   project_lvgl_xxx/
        lvgl/       
            ...
            ...
            ...
        my_src/
            lv_conf.h
            main.c
            mouse_cursor_icon.c

```

## 2.2 添加CMakeLists.txt

### 2.2.1  在根目录(project_lvgl_xxx)下添加CMakeLists.txt
在目录project_lvgl_xxx新建文本文件 CMakeLists.txt，其内容如下：
```sh

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


```


### 2.2.2 在子目录(project_lvgl_xxx/my_src)下添加CMakeLists.txt
在目录project_lvgl_xxx/my_src新建文本文件 CMakeLists.txt，其内容如下：
```sh
# 添加可执行文件
add_executable(main
    main.c
    mouse_cursor_icon.c
)

# 确保在链接前lv_conf.h已被复制
add_dependencies(main lvgl)

# 链接库
target_link_libraries(main
    lvgl
    lvgl::examples
    lvgl::demos
    lvgl::thorvg
    m
    pthread
)

# 添加自定义目标以便运行
add_custom_target(run 
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/main 
    DEPENDS main
)


```

## 2.3 修改LVGL的配置文件 my_src/lv_conf.h
修改内容如下：
```c++
/*Color depth: 8 (A8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888)*/
#define LV_COLOR_DEPTH 32


//...

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /*Size of the memory available for `lv_malloc()` in bytes (>= 2kB)*/
    #define LV_MEM_SIZE ( 10240 * 10240U)          /*[bytes]*/
//...


/*Default display refresh, input device read and animation step period.*/
#define LV_DEF_REFR_PERIOD  10      /*[ms]*/


#define LV_USE_ASSERT_STYLE         0   /*Check if the styles are properly initialized. (Very fast, recommended)*/
#define LV_USE_ASSERT_MEM_INTEGRITY 0   /*Check the integrity of `lv_mem` after critical operations. (Slow)*/
#define LV_USE_ASSERT_OBJ           0   /*Check the object's type and existence (e.g. not deleted). (Slow)*/



/* Add 2 x 32 bit variables to each lv_obj_t to speed up getting style properties */
#define LV_OBJ_STYLE_CACHE      20



/*API for open, read, etc*/
#define LV_USE_FS_POSIX 1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER 'A'   



/*Driver for evdev input devices*/
#define LV_USE_EVDEV    1

```

## 2.4 修改 project_lvgl_xxx/lvgl/src/drivers/evdev/lv_evdev.c
<!-- 仅针对一批触摸屏有问题的6818的开发板

> 对获取到的x, y进行等比缩放，因为显示屏800 * 480， 触摸屏 1024 * 600， 要把显示屏与触摸屏一一对应起来
```c++
第123、124行
            if(in.code == ABS_X || in.code == ABS_MT_POSITION_X) dsc->root_x = in.value*800/1024;
            else if(in.code == ABS_Y || in.code == ABS_MT_POSITION_Y) dsc->root_y = in.value*480/600;
​``` -->

```

## 2.5 修改main.c添加触摸屏
> ```
> 添加在案例窗口显示前面 lv_demo_widgets();
> lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");//RK3568是 event6 ,GEC6818是 event0
> 
> ```
>
> 

# 3. 编译
## 3.1 ubuntu下要安装cmake

cmake 是一个跨平台、自动构造工具，它的配置文件  CMakeLists.txt

命令如下 : //需要把网络改为 nat 模式

```sh
sudo snap install cmake --classic 
```
如果上述命令报错，如下是可能的解决方案之一:
```sh

# Ubuntu中snap install cmake报错解决方法（按步骤走）
# 1.更新系统包列表：
sudo apt update

# 2.升级所有已安装的包（包括snapd）：
sudo apt upgrade -y

# 3.单独更新snapd（确保获取最新版本）：
sudo apt install --only-upgrade snapd

# 4.刷新core snap（更新核心组件）：
sudo snap refresh core

# 5.重启系统（确保更新完全生效）：
sudo reboot

# 6.重新尝试安装CMake：
sudo snap install cmake --classic
```

## 3.2 编译步骤
前提：确保 整个project_lvgl_xxx在ubuntu的目录下或共享目录下。
然后如下是编译命令:

```sh
先cd到project_lvgl_xxx目录下

# 新建构建目录/build目录会保存生成的中间文件，该目录和里面的文件都可以删除如果后面的步骤错误，可以删除 build目录再从该步骤重新开始
mkdir build
cd build

# 生成Makefile
cmake ..

# 构建项目，第一次make需要几分钟，耐心等待，后面的make比较快
# 如果成功了，会在 project_lvgl_xxx目录下生成一个子目录 bin
# 程序文件 main 就在 bin目录中
make

注意：
后续如果更改了配置文件(如:lv_conf.h , CMakeLists.txt)，就需要重新从 cmake ..  步骤执行

后续如果更改了.c文件，直接make即可
```

