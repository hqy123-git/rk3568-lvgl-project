#ifndef __SNAKE_H__
#define __SNAKE_H__
#include "lvgl.h"
#include <stdlib.h>
#include <time.h>
// 游戏常量定义
#define GRID_SIZE 20   //单个格子大小
#define GRID_WIDTH 35  // 35列
#define GRID_HEIGHT 30 // 29行
#define SNAKE_GAME_AREA_WIDTH (GRID_WIDTH * GRID_SIZE)
#define SNAKE_GAME_AREA_HEIGHT (GRID_HEIGHT * GRID_SIZE)

// 游戏状态枚举
typedef enum {
    SNAKE_GAME_READY, //初始化完成，等待开始
    SNAKE_GAME_START,//游戏中
    SNAKE_GAME_PAUSED,//暂停
    SNAKE_GAME_OVER//游戏结束
} SnakeGameState;

// 方向枚举
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// 游戏结构体
typedef struct {
    lv_obj_t *game_area;
    lv_obj_t *score_label; //用来显示当前分数的标签
    lv_obj_t *high_score_label;//用来显示最高分数的标签
    lv_obj_t *state_label;//用来显示游戏状态的标签

    lv_obj_t *snake_parts[GRID_WIDTH * GRID_HEIGHT];  // 蛇身各部分的LVGL对象指针
    lv_point_t snake_pos[GRID_WIDTH * GRID_HEIGHT];    // 蛇身各部分在网格中的坐标
    int snake_length;                                 // 蛇的当前长度（单位：格）
    lv_obj_t *food_obj; //食物 指针
    lv_point_t food_pos; //食物坐标

    Direction current_dir;//当前的前进方向
    Direction next_dir;//下一个方向，也就是手指滑动屏幕的方向
    int score;//当前分数
    int high_score;//最高分数
    SnakeGameState state;//游戏状态
    lv_timer_t *game_timer;//游戏定时器
    int period;//运动一步的时间，单位为毫秒
} SnakeGame;

// 初始化游戏
void snake_game_init() ;

#endif