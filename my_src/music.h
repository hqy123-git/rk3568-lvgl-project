#ifndef MUSIC_H
#define MUSIC_H

#include "lvgl/lvgl.h"

// 音乐播放软件结构体
typedef struct {
    lv_obj_t *screen;           // 音乐播放软件屏幕
    lv_obj_t *back_button;      // 返回主菜单按钮
    lv_obj_t *music_buttons[3]; // 三个音乐按钮
    int current_playing;        // 当前播放的音乐索引（-1表示未播放）
} MusicPlayer;

// 函数声明
void music_player_init();          // 初始化音乐播放软件 
void play_music(int music_index);  // 播放指定音乐
void stop_music();                // 停止当前音乐

#endif /* MUSIC_H */