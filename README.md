# RK3568 LVGL 多媒体娱乐平台

基于 RK3568 Linux 开发板与 LVGL v8 图形库打造的嵌入式多媒体交互系统，集成多款经典休闲游戏与音乐播放器，支持触摸屏操作。

---

## 功能特性

### 游戏模块
| 游戏 | 核心特性 |
|------|----------|
| **2048** | 4×4 矩阵滑动合并算法、分数系统、胜负判定、登录验证 |
| **贪吃蛇** | 链表蛇身管理、自动移动、食物随机生成、碰撞检测、暂停/恢复 |
| **五子棋** | 15×15 棋盘、黑白双方落子、四方向五连珠判定 |
| **雷霆战机** | 对象池管理子弹/敌机、AABB 碰撞检测、自动射击、生命值系统 |
| **中国象棋** | 完整棋子移动规则、红黑双方对战、悔棋/重开功能 |

### 音乐播放器
- 基于 MPlayer 子进程实现音乐播放
- 通过命名管道（FIFO）控制播放/暂停
- 支持多首音乐切换

### 交互特性
- 触摸屏滑动手势控制（2048 游戏）
- 按钮点击事件响应
- 多场景平滑切换（主菜单 → 游戏/音乐 → 返回）

---

## 技术栈

- **编程语言**：C
- **操作系统**：Linux（RK3568 平台）
- **GUI 框架**：LVGL v8
- **构建系统**：CMake
- **音乐播放**：MPlayer
- **显示驱动**：FrameBuffer（/dev/fb0）
- **输入驱动**：evdev（/dev/input/event6）
- **字体支持**：思源黑体、思源宋体

---

## 项目结构

```
project_lvgl_myproject/
├── CMakeLists.txt          # 顶层 CMake 配置（交叉编译）
├── bin/                    # 可执行文件输出目录
│   └── main
├── build/                  # 构建产物目录
├── lvgl/                   # LVGL 库源码
└── my_src/                 # 用户源代码
    ├── CMakeLists.txt      # 模块构建配置
    ├── main.c              # 主程序入口
    ├── game2048.c/h        # 2048 游戏
    ├── snake.c/h           # 贪吃蛇游戏
    ├── fivegame.c/h        # 五子棋游戏
    ├── thunder.c/h         # 雷霆战机游戏
    ├── chinese_chess.c/h   # 中国象棋游戏
    ├── music.c/h           # 音乐播放器
    ├── SiYuanHeiTi.c       # 思源黑体字体
    ├── SiYuanSongTi.c      # 思源宋体字体
    └── lv_conf.h           # LVGL 配置文件
```

---

## 硬件要求

- RK3568 Linux 开发板
- 触摸屏（evdev 输入设备）
- 显示屏（FrameBuffer 输出）
- 支持 MP3/OGG 格式的音频文件
- MPlayer 播放器（需预装）

---

## 编译与部署

### 1. 环境准备

在 Ubuntu 虚拟机中安装交叉编译工具链：

```bash
# 安装 ARM 交叉编译器
sudo apt-get install gcc-aarch64-linux-gnu

# 或使用厂商工具链（项目中使用的是 linaro-7.5.0）
# /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/
```

### 2. 修改编译配置

根据实际交叉编译器路径修改 `CMakeLists.txt`：

```cmake
set(CMAKE_C_COMPILER /path/to/aarch64-linux-gnu/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER /path/to/aarch64-linux-gnu/bin/arm-linux-g++)
```

### 3. 编译项目

```bash
mkdir build && cd build
cmake ..
make
```

### 4. 部署到开发板

```bash
# 将编译产物传输到 RK3568 开发板
scp bin/main root@<RK3568_IP>:/home/root/

# 在开发板上执行
cd /home/root
chmod +x main
./main
```

---

## 核心实现

### LVGL 硬件适配

```c
// main.c - LVGL 初始化
lv_display_t *disp = lv_linux_fbdev_create();
lv_linux_fbdev_set_file(disp, "/dev/fb0");
lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
```

### 系统主循环

```c
// main.c - LVGL 事件处理循环
while(1) {
    lv_timer_handler();   // 处理 LVGL 定时器任务
    usleep(5000);         // 5ms 延时
}
```

### 音乐播放控制

```c
// music.c - 通过 MPlayer 子进程播放音乐
system("mplayer -quiet -slave -loop 0 -input file=/my_fifo music.mp3 &");

// 通过命名管道控制暂停/播放
int fd = open("/my_fifo", O_RDWR);
write(fd, "pause\n", 6);
close(fd);
```

### 2048 滑动合并算法

```c
// game2048.c - 向左移动合并
int move_left() {
    for (int i = 0; i < GRID_SIZE; i++) {
        int pos = 0;
        int merged[GRID_SIZE] = {0};
        for (int j = 0; j < GRID_SIZE; j++) {
            if (game_grid[i][j] == 0) continue;
            if (pos > 0 && game_grid[i][pos-1] == game_grid[i][j] && !merged[pos-1]) {
                game_grid[i][pos-1] *= 2;
                score += game_grid[i][pos-1];
                merged[pos-1] = 1;
                game_grid[i][j] = 0;
            } else {
                if (j != pos) {
                    game_grid[i][pos] = game_grid[i][j];
                    game_grid[i][j] = 0;
                }
                pos++;
            }
        }
    }
}
```

### 贪吃蛇定时器驱动

```c
// snake.c - 游戏定时器控制
game.game_timer = lv_timer_create(game_period, snake_update_cb);
lv_timer_set_period(game.game_timer, 250);  // 250ms 更新一次
lv_timer_resume(game.game_timer);
```

---

## 游戏操作说明

| 游戏 | 操作方式 |
|------|----------|
| 2048 | 触摸屏滑动（上下左右） |
| 贪吃蛇 | 方向按钮点击 |
| 五子棋 | 点击棋盘落子 |
| 雷霆战机 | 左右移动按钮 + 射击按钮 |
| 中国象棋 | 点击棋子选中，再点击目标位置移动 |
| 音乐 | 点击音乐按钮播放/暂停 |

---

## 目录约定

- 图片资源放置在开发板根目录（`/home/root/`）
- 音乐文件支持 MP3、OGG 格式
- 字体文件已编译进程序

---

## 许可证

本项目仅供学习交流使用。

---

## 致谢

- [LVGL](https://lvgl.io/) - 开源图形库
- [MPlayer](http://www.mplayerhq.hu/) - 开源媒体播放器
