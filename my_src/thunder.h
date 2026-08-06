#ifndef __THUNDER_H__
#define __THUNDER_H__

#include "lvgl/lvgl.h"

// 游戏常量定义
#define THUNDER_WIDTH 800
#define THUNDER_HEIGHT 500
#define THUNDER_GAME_AREA_WIDTH 600
#define THUNDER_GAME_AREA_HEIGHT 500
#define CONTROL_BUTTON_WIDTH 80
#define CONTROL_BUTTON_HEIGHT 80
#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define BULLET_WIDTH 4
#define BULLET_HEIGHT 12
#define ENEMY_WIDTH 30
#define ENEMY_HEIGHT 30
#define MAX_BULLETS 50
#define MAX_ENEMIES 20
#define ENEMY_SPEED 2
#define BULLET_SPEED 8
#define PLAYER_SPEED 30
#define AUTO_FIRE_INTERVAL 50  // 自动射击间隔（毫秒）

// 游戏对象结构体
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int speed;
    int active;
    lv_obj_t *obj;
} GameObject;

// 玩家结构体
typedef struct {
    GameObject obj;
    int score;
    int lives;
} Player;

// 游戏状态
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *game_container;
    lv_obj_t *score_label;
    lv_obj_t *lives_label;
    lv_obj_t *left_button;
    lv_obj_t *right_button;
    lv_obj_t *fire_button;
    Player player;
    GameObject bullets[MAX_BULLETS];
    GameObject enemies[MAX_ENEMIES];
    lv_timer_t *game_timer;
    lv_timer_t *auto_fire_timer;
    int game_running;
    int auto_fire_enabled;
} ThunderGame;

// 函数声明
void thunder_game_init();
void thunder_game_event_cb(lv_event_t *e);
void go_back_from_thunder(lv_event_t *e);

#endif