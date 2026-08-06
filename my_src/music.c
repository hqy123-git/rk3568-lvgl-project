#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "music.h"
#include <errno.h>
// 全局变量
static MusicPlayer player;
lv_obj_t *man_src;
int pause_music;

// 音乐文件路径
const char *music_files[] = {
    "./man.mp3",
    "./3.ogg",
    "./3.ogg"
};

// 播放音乐函数
void play_music(int music_index) {
    if (music_index < 0 || music_index >= 3) return;
    
    // 停止当前播放的音乐
    stop_music();
    
    // 设置当前播放音乐索引
    player.current_playing = music_index;
    pid_t pid = fork();
    if (pid == 0) 
    {
        // 子进程执行mplayer
        // 构建mplayer命令（后台播放）
        char cmd[100];
        sprintf(cmd, "mplayer -quiet -slave -loop 0 -input file=/my_fifo %s &", music_files[music_index]);
    
        // 执行命令
        system(cmd);
        exit(1);
    } 
    else if (pid < 0) 
    {
        // fork失败，输出错误信息
        perror("fork");
    }
   
    
    // 更新按钮样式
    for (int i = 0; i < 3; i++) {
        if (i == music_index) {
            lv_obj_set_style_bg_color(player.music_buttons[i], lv_color_hex(0x00FF00), 0); // 播放中为绿色
        } else {
            lv_obj_set_style_bg_color(player.music_buttons[i], lv_color_hex(0x817F26), 0); // 其他为默认色
        }
    }
}

// 停止音乐函数
void stop_music() {
    // 如果有音乐在播放
    if (player.current_playing >= 0) 
    {
        // 停止进程
        system("killall mplayer");
        // 重置按钮样式
        for (int i = 0; i < 3; i++) 
        {
            lv_obj_set_style_bg_color(player.music_buttons[i], lv_color_hex(0x817F26), 0);
        }
        
        // 重置当前播放索引
        player.current_playing = -1;
    }
}

// 音乐按钮事件处理
static void music_button_event_cb(lv_event_t *e) {
    lv_obj_t *button = lv_event_get_target(e);
    
    // 查找是哪个音乐按钮被点击
    for (int i = 0; i < 3; i++) 
    {
        if (button == player.music_buttons[i]) 
        {
            // 如果点击的是正在播放的音乐，则暂停/继续播放
            if (player.current_playing == i) 
            {
                if(pause_music==0)
                {
                    printf("暂停\n");
                    pause_music=1;
                }
                else
                {
                    printf("继续播放\n");
                    pause_music=0;
                }
                // 使用统一的管道路径（与play_music函数中一致）
                int fd = open("/my_fifo", O_RDWR);
                if (fd >= 0) 
                {
                    write(fd, "pause\n", 6);
                    close(fd);
                } else {
                    // 输出调试信息
                    printf("Failed to open fifo: %s\n", strerror(errno));
                }
            } 
            else 
            {
                // 否则播放选中的音乐
                play_music(i);
            }
            break;
        }
    }
}

// 返回主菜单按钮事件处理
static void back_button_event_cb(lv_event_t *e) {
    // 停止音乐
    stop_music();
    // 返回主菜单
    lv_screen_load(man_src);
    lv_obj_del(player.screen);
    
}

// 音乐播放软件初始化
void music_player_init() {
    // 创建音乐播放软件屏幕
    player.screen = lv_obj_create(NULL);
    lv_screen_load(player.screen);
    
    // 设置屏幕背景颜色
    lv_obj_set_style_bg_color(player.screen, lv_color_hex(0x000033), 0);
    
    // 初始化当前播放索引
    player.current_playing = -1;
    pause_music=0;
    // 创建返回主菜单按钮
    player.back_button = lv_button_create(player.screen);
    lv_obj_set_size(player.back_button, 120, 50);
    lv_obj_set_align(player.back_button, LV_ALIGN_TOP_RIGHT);
    lv_obj_t *back_label = lv_label_create(player.back_button);
    lv_label_set_text(back_label, "ESC");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_20, 0);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(player.back_button, back_button_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 创建音乐标题
    lv_obj_t *title = lv_label_create(player.screen);
    lv_label_set_text(title, "Music Player");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 50);
    
    // 音乐按钮宽度和高度
    int button_width = 200;
    int button_height = 80;
    int button_spacing = 40;
    
    // 创建三个音乐按钮
    for (int i = 0; i < 3; i++) {
        // 创建按钮
        player.music_buttons[i] = lv_button_create(player.screen);
        lv_obj_set_size(player.music_buttons[i], button_width, button_height);
        
        // 计算按钮位置（居中排列）
        int x = (lv_obj_get_width(player.screen) - button_width) / 2;
        int y = 150 + i * (button_height + button_spacing);
        lv_obj_set_pos(player.music_buttons[i], x, y);
        
        // 设置按钮颜色
        lv_obj_set_style_bg_color(player.music_buttons[i], lv_color_hex(0x817F26), 0);
        
        // 创建按钮标签
        lv_obj_t *label = lv_label_create(player.music_buttons[i]);
        char text[20];
        sprintf(text, "Music %d", i + 1);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_center(label);
        
        // 添加按钮事件
        lv_obj_add_event_cb(player.music_buttons[i], music_button_event_cb, LV_EVENT_CLICKED, NULL);
    }
}