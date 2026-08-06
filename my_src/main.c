#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "game2048.h"
#include "snake.h"
#include "fivegame.h"
#include "thunder.h"
#include "music.h"
#include "chinese_chess.h"

lv_obj_t *man_src=NULL;

void game_2048_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        into_screeen();
    }
}


void game_snake_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        snake_game_init();
    }
}

void game_five_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        fivegame_init();
    }
}

void game_thunder_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        thunder_game_init();
    }
}

// 主屏幕音乐按钮事件处理
void game_music_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        music_player_init();
    }
}

// 主屏幕象棋按钮事件处理
void game_chess_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) 
    {
        // 初始化游戏
        chess_game_init();
        
        // 创建游戏UI
        chess_game_create_ui();
    }
}
//创建一个图片
void image1()
{
    lv_obj_t *image=lv_image_create(man_src);
    lv_obj_set_pos(image,0,0);
    lv_image_set_src(image,"A:./2.jpeg");//图片的地址
}

void create_man_src()
{
    man_src=lv_obj_create(NULL);
    lv_screen_load(man_src);
    image1();
    //
    lv_obj_t *game_2048=lv_button_create(man_src);
    lv_obj_set_size(game_2048,80,80);
    lv_obj_set_pos(game_2048,20,20);
    lv_obj_set_style_bg_color(game_2048, lv_color_hex(0x817F26), 0);
    lv_obj_t * label = lv_label_create(game_2048);
    lv_label_set_text(label, "2048");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(game_2048,game_2048_event_cb,LV_EVENT_CLICKED,NULL);
    //
    lv_obj_t *game_snake=lv_button_create(man_src);
    lv_obj_set_size(game_snake,80,80);
    lv_obj_set_pos(game_snake,120,20);
    lv_obj_set_style_bg_color(game_snake, lv_color_hex(0x817F26), 0);
    lv_obj_t * label1 = lv_label_create(game_snake);
    lv_label_set_text(label1, "snake");
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_20, 0);
    lv_obj_center(label1);
    lv_obj_add_event_cb(game_snake,game_snake_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *game_five=lv_button_create(man_src);
    lv_obj_set_size(game_five,80,80);
    lv_obj_set_pos(game_five,240,20);
    lv_obj_set_style_bg_color(game_five, lv_color_hex(0x817F26), 0);
    lv_obj_t * label2 = lv_label_create(game_five);
    lv_label_set_text(label2, "five");
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_20, 0);
    lv_obj_center(label2);
    lv_obj_add_event_cb(game_five,game_five_event_cb,LV_EVENT_CLICKED,NULL);
    
    lv_obj_t *game_thunder=lv_button_create(man_src);
    lv_obj_set_size(game_thunder,80,80);
    lv_obj_set_pos(game_thunder,340,20);
    lv_obj_set_style_bg_color(game_thunder, lv_color_hex(0x817F26), 0);
    lv_obj_t * label3 = lv_label_create(game_thunder);
    lv_label_set_text(label3, "thunder");
    lv_obj_set_style_text_font(label3, &lv_font_montserrat_20, 0);
    lv_obj_center(label3);
    lv_obj_add_event_cb(game_thunder,game_thunder_event_cb,LV_EVENT_CLICKED,NULL);
    
    // 音乐播放软件按钮
    lv_obj_t *game_music=lv_button_create(man_src);
    lv_obj_set_size(game_music,80,80);
    lv_obj_set_pos(game_music,440,20);
    lv_obj_set_style_bg_color(game_music, lv_color_hex(0x817F26), 0);
    lv_obj_t * label4 = lv_label_create(game_music);
    lv_label_set_text(label4, "music");
    lv_obj_set_style_text_font(label4, &lv_font_montserrat_20, 0);
    lv_obj_center(label4);
    lv_obj_add_event_cb(game_music,game_music_event_cb,LV_EVENT_CLICKED,NULL);
    
    // 中国象棋游戏按钮
    lv_obj_t *game_chess=lv_button_create(man_src);
    lv_obj_set_size(game_chess,80,80);
    lv_obj_set_pos(game_chess,540,20);
    lv_obj_set_style_bg_color(game_chess, lv_color_hex(0x817F26), 0);
    lv_obj_t * label5 = lv_label_create(game_chess);
    lv_label_set_text(label5, "chess");
    lv_obj_set_style_text_font(label5, &lv_font_montserrat_20, 0);
    lv_obj_center(label5);
    lv_obj_add_event_cb(game_chess,game_chess_event_cb,LV_EVENT_CLICKED,NULL);
}

int main(void)
{
    lv_init();

    /*Linux frame buffer device init*/
    //初始化帧缓冲设备---》显示屏
    lv_display_t * disp = lv_linux_fbdev_create();//创建一个默认屏幕，并设置为其“Active Screen”
    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");//RK3568是 event6 ,GEC6818是 event0
    /*Create a Demo*/
    //lv_demo_widgets();
    //lv_demo_widgets_start_slideshow();
    /*Handle LVGL tasks*/
    create_man_src();
    
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}