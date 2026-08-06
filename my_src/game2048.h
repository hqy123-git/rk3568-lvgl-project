#ifndef __game2048_h__
#define __game2048_h__
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"


void textarea_event_cb(lv_event_t *e);//文本框回调函数
void ok_button_event_cb(lv_event_t *e);//消息框的按钮回调函数
void button_event_cb(lv_event_t *e);//进入游戏界面按钮回调函数
void create_grid();// 创建游戏网格
void update_grid();// 更新网格显示
void update_score();// 更新分数显示
void spawn_random_tile();// 在空白位置随机生成 2 或 4
int move_left();// 向左移动并合并
int move_right();// 向右移动并合并
int move_up();// 向上移动并合并
int move_down();// 向下移动并合并
void esc_but_event_cb(lv_event_t *e);//回到登录界面
void again_but_event_cb(lv_event_t *e);//重新开始回调函数
void scr_dir_event_cb(lv_event_t *e);//发生滑动事件回调函数
void game_over_msg(int a);//创建游戏结束消息框
int game_over();//判断游戏是否结束，结束返回1(胜利),-1(失败)，否返回0
void game_init();//初始化游戏界面
void into_screeen();//登录界面


#endif