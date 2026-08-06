#ifndef __fivegame_h__
#define __fivegame_h__

// 五子棋UI相关常量
#define GOMOKU_BOARD_SIZE 15          // 15*15的棋盘
#define GOMOKU_CELL_SIZE 36           // 每个格子36x36像素
#define GOMOKU_BOARD_WIDTH (GOMOKU_BOARD_SIZE * GOMOKU_CELL_SIZE)
#define GOMOKU_BOARD_HEIGHT (GOMOKU_BOARD_SIZE * GOMOKU_CELL_SIZE)

// 颜色定义
#define GOMOKU_BOARD_COLOR 0xFFE6B3   // 棋盘颜色（浅黄色）
#define GOMOKU_LINE_COLOR 0x000000    // 线条颜色（黑色）
#define GOMOKU_BLACK_COLOR 0x000000   // 黑棋颜色
#define GOMOKU_WHITE_COLOR 0xFFFFFF   // 白棋颜色
#define GOMOKU_HIGHLIGHT_COLOR 0xFF0000 // 高亮颜色（红色）

void fivegame_init();

#endif