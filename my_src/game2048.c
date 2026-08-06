#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "game2048.h"
#include <string.h>

static lv_obj_t *screen=NULL;
static lv_obj_t *screen1=NULL;
extern lv_obj_t *man_src;

lv_obj_t *textarea1=NULL;
lv_obj_t *textarea2=NULL;
void textarea_event_cb(lv_event_t *e)
{
    lv_obj_t *ta=lv_event_get_target(e);
    lv_obj_t *kb=(lv_obj_t *)lv_event_get_user_data(e);
    lv_event_code_t code=lv_event_get_code(e);
    if(code == LV_EVENT_FOCUSED)
    {
        lv_obj_remove_flag(kb,LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb,ta);
    }
    else if(code ==LV_EVENT_DEFOCUSED)
    {
        lv_obj_add_flag(kb,LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb,NULL);
    }
}
void ok_button_event_cb(lv_event_t *e)
{
    //获取发生事件按钮的地址
    lv_obj_t *button=lv_event_get_target(e);
    //获取按钮的标签
    lv_obj_t *label=lv_obj_get_child(button,0);
    //获取标签内容
    const char *text=lv_label_get_text(label);
     //获取消息框地址
    lv_obj_t *msg=(lv_obj_t *)lv_event_get_user_data(e);
    if(strcmp(text,"ok") == 0)
    {
        //是ok按钮
        lv_msgbox_close(msg);
    }
    else if(strcmp(text,"close") == 0)
    {
        //是close按钮
        lv_msgbox_close(msg);
    }
   
    
}

void button_event_cb(lv_event_t *e)
{

    const char *s1=lv_textarea_get_text(textarea1);
    const char *s2=lv_textarea_get_text(textarea2);
    int a =strncmp(s1,"hqy",3) + strncmp(s2,"123456",6);

    if(a == 0)
    {
        printf("登录成功\n");
        game_init();
    }
    else
    {
        printf("登录失败\n");
        lv_obj_t *msgbox=lv_msgbox_create(screen);
        lv_msgbox_add_close_button(msgbox);
        lv_msgbox_add_title(msgbox,"warning");
        lv_msgbox_add_text(msgbox,"name and mima error");
        lv_obj_t *ok_button = lv_msgbox_add_footer_button(msgbox,"ok");
        lv_obj_t *close_button = lv_msgbox_add_footer_button(msgbox,"close");
        lv_obj_add_event_cb(ok_button,ok_button_event_cb,LV_EVENT_CLICKED,msgbox);
        lv_obj_add_event_cb(close_button,ok_button_event_cb,LV_EVENT_CLICKED,msgbox);
    }
}

#define GRID_SIZE 4 
static int game_grid[GRID_SIZE][GRID_SIZE] = {0};//保存每格的值
static lv_obj_t *grid_obj;
static lv_obj_t *label_score;
static int score = 0;

// 创建游戏网格
void create_grid() 
{
    grid_obj = lv_obj_create(screen1);//创建游戏区域
    lv_obj_set_size(grid_obj, 580, 580);
    lv_obj_set_pos(grid_obj,10,10);
    lv_obj_set_style_bg_color(grid_obj, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_radius(grid_obj, 6, 0);
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        for (int j = 0; j < GRID_SIZE; j++) 
        {
            lv_obj_t *cell = lv_obj_create(grid_obj);//创建每一格
            lv_obj_set_size(cell, 125, 125);
            lv_obj_set_pos(cell, j * 135, i * 135);
            lv_obj_set_style_bg_color(cell, lv_color_hex(0xCDCDCD), 0);
            lv_obj_set_style_radius(cell, 3, 0);

            lv_obj_t *num_label = lv_label_create(cell);
            lv_label_set_text(num_label, "");
            lv_obj_center(num_label);
            lv_obj_set_style_text_color(num_label, lv_color_hex(0x776E65), 0);
            lv_obj_set_style_text_font(num_label, &lv_font_montserrat_24, 0);
        }
    }
}

// 更新网格显示
void update_grid() 
{
    int count=0;
    //获取游戏区域每一个格子
    lv_obj_t *children = lv_obj_get_child(grid_obj, count);
    int idx = 0;
    while (1) //直到获取的格子为空，也就是说格子全部获取完了
    {
        int i = idx / GRID_SIZE;
        int j = idx % GRID_SIZE;
        lv_obj_t *num_label = lv_obj_get_child(children, 0);
        if (game_grid[i][j] == 0) 
        {
            lv_label_set_text(num_label, "");
            lv_obj_set_style_bg_color(children, lv_color_hex(0xCDCDCD), 0);
        } 
        else 
        {
            char buf[10];
            //将每格数据保存在数组buf中
            sprintf(buf, "%d", game_grid[i][j]);
            //更新格子数据显示
            lv_label_set_text(num_label, buf);
            // 根据数值设置不同背景色，这里简化处理
            if (game_grid[i][j] == 2) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEEE4DA), 0);
            else if (game_grid[i][j] == 4) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDE0C8), 0);
            else if (game_grid[i][j] == 8) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xF2B179), 0);
            else if (game_grid[i][j] == 16) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xF59563), 0);
            else if (game_grid[i][j] == 32) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xF67C5F), 0);
            else if (game_grid[i][j] == 64) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xF65E3B), 0);
            else if (game_grid[i][j] == 128) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDCF72), 0);
            else if (game_grid[i][j] == 256) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDCC61), 0);
            else if (game_grid[i][j] == 512) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDC850), 0);
            else if (game_grid[i][j] == 1024) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDC53F), 0);
            else if (game_grid[i][j] == 2048) 
            lv_obj_set_style_bg_color(children, lv_color_hex(0xEDC22E), 0);
            //设置数字颜色
            lv_obj_set_style_text_color(num_label, (game_grid[i][j] >= 8) ? lv_color_hex(0xF9F6F2) : lv_color_hex(0x776E65), 0);
        }
        count++;
        if(count == 16)
        {
            break;
        }
        children = lv_obj_get_child(grid_obj, count);
        idx++;
    }
}
// 更新分数显示
void update_score() 
{
    char buf[20];
    sprintf(buf, "Score: %d", score);
    lv_label_set_text(label_score, buf);
}

// 在空白位置随机生成 2 或 4
void spawn_random_tile() 
{
    int empty_cells[GRID_SIZE * GRID_SIZE][2];//记录格子为0的格子的x（0列为x），y轴坐标（1列为y）
    int count = 0;//记录为0的格子数
    //遍历每个格子
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        for (int j = 0; j < GRID_SIZE; j++) 
        {
            //如果为0，就保存这个格子的坐标
            if (game_grid[i][j] == 0) 
            {
                empty_cells[count][0] = i;
                empty_cells[count][1] = j;
                count++;
            }
        }
    }
    if (count == 0) 
        return ;
    int rnd = rand() % count;
    int val = (rand() % 10 == 0) ? 4 : 2;
    game_grid[empty_cells[rnd][0]][empty_cells[rnd][1]] = val;
}

// 向左移动并合并
int move_left() 
{
    int changed = 0;//代表网格中数字是否发生改变
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        int pos = 0;//代表要将要发生合并或者不为0的格子的后一列的y轴
        int merged[GRID_SIZE] = {0};//保存每一个格子是否合并，0没有合并，1合并
        for (int j = 0; j < GRID_SIZE; j++) 
        {
            if (game_grid[i][j] == 0) 
                continue;
            //当前j与pos的前一列相等，并且没有合并过
            if (pos > 0 && game_grid[i][pos - 1] == game_grid[i][j] && !merged[pos - 1]) 
            {
                game_grid[i][pos - 1] *= 2;
                score += game_grid[i][pos - 1];
                merged[pos - 1] = 1;
                game_grid[i][j] = 0;
                changed = 1;
            } 
            else 
            {
                //代表第pos列的格子为0
                if (j != pos) 
                {
                    game_grid[i][pos] = game_grid[i][j];
                    game_grid[i][j] = 0;
                    changed = 1;
                }
                pos++;
            }
        }
    }
    return changed;
}

// 向右移动并合并
int move_right() 
{
    int changed = 0;
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        int pos = GRID_SIZE - 1;
        int merged[GRID_SIZE] = {0};
        for (int j = GRID_SIZE - 1; j >= 0; j--) 
        {
            if (game_grid[i][j] == 0) 
                continue;
            if (pos < GRID_SIZE - 1 && game_grid[i][pos + 1] == game_grid[i][j] && !merged[pos + 1]) 
            {
                game_grid[i][pos + 1] *= 2;
                score += game_grid[i][pos + 1];
                merged[pos + 1] = 1;
                game_grid[i][j] = 0;
                changed = 1;
            } 
            else 
            {
                if (j != pos) 
                {
                    game_grid[i][pos] = game_grid[i][j];
                    game_grid[i][j] = 0;
                    changed = 1;
                }
                pos--;
            }
        }
    }
    return changed;
}

// 向上移动并合并
int move_up() 
{
    int changed = 0;
    for (int j = 0; j < GRID_SIZE; j++) 
    {
        int pos = 0;
        int merged[GRID_SIZE] = {0};
        for (int i = 0; i < GRID_SIZE; i++) 
        {
            if (game_grid[i][j] == 0) 
                continue;
            if (pos > 0 && game_grid[pos - 1][j] == game_grid[i][j] && !merged[pos - 1]) 
            {
                game_grid[pos - 1][j] *= 2;
                score += game_grid[pos - 1][j];
                merged[pos - 1] = 1;
                game_grid[i][j] = 0;
                changed = 1;
            } 
            else 
            {
                if (i != pos) 
                {
                    game_grid[pos][j] = game_grid[i][j];
                    game_grid[i][j] = 0;
                    changed = 1;
                }
                pos++;
            }
        }
    }
    return changed;
}

// 向下移动并合并
int move_down() 
{
    int changed = 0;
    for (int j = 0; j < GRID_SIZE; j++) 
    {
        int pos = GRID_SIZE - 1;
        int merged[GRID_SIZE] = {0};
        for (int i = GRID_SIZE - 1; i >= 0; i--) 
        {
            if (game_grid[i][j] == 0) 
                continue;
            if (pos < GRID_SIZE - 1 && game_grid[pos + 1][j] == game_grid[i][j] && !merged[pos + 1]) 
            {
                game_grid[pos + 1][j] *= 2;
                score += game_grid[pos + 1][j];
                merged[pos + 1] = 1;
                game_grid[i][j] = 0;
                changed = 1;
            } else 
            {
                if (i != pos) 
                {
                    game_grid[pos][j] = game_grid[i][j];
                    game_grid[i][j] = 0;
                    changed = 1;
                }
                pos--;
            }
        }
    }
    return changed;
}
//回到登录界面
void esc_but_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        // 切换到screen
        system("killall mplayer");
        lv_screen_load(screen);
        lv_obj_del(screen1);
    }

}
//重新开始回调函数
void again_but_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        //清空数据
        lv_label_set_text(label_score, "Score: 0");
        int idx=0;
        int count=0;
        //获取游戏区域每一个格子
        lv_obj_t *children = lv_obj_get_child(grid_obj, count);
        while (1) //直到获取的格子为空，也就是说格子全部获取完了
        {
            int i = idx / GRID_SIZE;
            int j = idx % GRID_SIZE;
            lv_obj_t *num_label = lv_obj_get_child(children, 0);
            if (game_grid[i][j] != 0) 
            {
                lv_label_set_text(num_label, "");
                lv_obj_set_style_bg_color(children, lv_color_hex(0xCDCDCD), 0);
                game_grid[i][j] = 0;
            } 
            count++;
            if(count == 16)
            {
                break;
            }
            children = lv_obj_get_child(grid_obj, count);
            idx++;
        } 
        spawn_random_tile();
        spawn_random_tile();
        update_grid();
    }
}
//判断游戏是否结束，结束返回1，否返回0
int game_over()
{
    // 检查横向（左右相邻）是否可以合并
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        for (int j = 0; j < GRID_SIZE - 1; j++) 
        {
            if(game_grid[i][j] == 2048)
            {
                return 1;
            }
            if(game_grid[i][j] == 0)
            {
                return 0;
            }
            if (game_grid[i][j] != 0 && game_grid[i][j] == game_grid[i][j + 1]) 
            {
                return 0; // 发现一对可横向合并的方块
            }
        }
    }
    
    // 检查纵向（上下相邻）是否可以合并
    for (int i = 0; i < GRID_SIZE - 1; i++) 
    {
        for (int j = 0; j < GRID_SIZE; j++) 
        {
            if(game_grid[i][j] == 2048)
            {
                return 1;
            }
            if(game_grid[i][j] == 0)
            {
                return 0;
            }
            if (game_grid[i][j] != 0 && game_grid[i][j] == game_grid[i + 1][j]) 
            {
                return 0; // 发现一对可纵向合并的方块
            }
        }
    }
    return -1;
}
//游戏结束消息框
void game_over_msg(int a)
{
    lv_obj_t *msg=lv_msgbox_create(screen1);
    lv_msgbox_add_close_button(msg);
    lv_msgbox_add_title(msg,"Hint");
    if(a == 1)
    {
        lv_msgbox_add_text(msg,"game Victory!");
    }
    if(a == -1)
    {
        lv_msgbox_add_text(msg,"game over!");
    }
}

//发生滑动事件回调函数
void scr_dir_event_cb(lv_event_t *e)
{
    int mov=0;
    lv_event_code_t code = lv_event_get_code(e);
    // 只在拖动结束时获取手势方向
    if (code == LV_EVENT_GESTURE) 
    {
        // 获取手势
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(lv_indev_active());

        // 执行相应操作
        if (gesture_dir == LV_DIR_LEFT) 
        {
            //执行左滑操作
            mov=move_left();
        } 
        else if (gesture_dir == LV_DIR_RIGHT) 
        {
            //执行右滑操作
            mov=move_right(); 
        }
        else if(gesture_dir == LV_DIR_TOP)
        {
            //上滑
            mov=move_up();
        }
        else if(gesture_dir == LV_DIR_BOTTOM)
        {
            //下滑
            mov=move_down();
        }
    }
    if(mov !=0 )
    {
        update_score();
        spawn_random_tile();
        update_grid();
        //判断游戏是否结束
        int a=game_over();
        if(-1 == a)
        {
            //失败
            game_over_msg(a);
        }
        if(1 == a)
        {
            //胜利
            game_over_msg(a);
        }
    }
}


//初始化游戏界面
void game_init()
{
    // 初始化随机数种子
    srand(time(NULL));
    //游戏界面
    screen1=lv_obj_create(NULL);
    lv_screen_load(screen1);
   
    // 创建游戏网格
    create_grid(); 
    //创建计分板
    label_score=lv_label_create(screen1);
    lv_label_set_text(label_score, "Score: 0");
    lv_obj_set_style_text_font(label_score, &lv_font_montserrat_30, 0);
    lv_obj_align(label_score, LV_ALIGN_TOP_MID, 200, 50);

    //创建返回，重新开始按钮
    lv_obj_t *esc_but=lv_button_create(screen1);
    lv_obj_set_size(esc_but,200,80);
    lv_obj_set_style_bg_color(esc_but, lv_color_hex(0x817F26), 0);
    lv_obj_align(esc_but, LV_ALIGN_BOTTOM_RIGHT,-100,-100);
    lv_obj_t * label = lv_label_create(esc_but);
    lv_label_set_text(label, "esc");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(esc_but,esc_but_event_cb,LV_EVENT_CLICKED,NULL);

    lv_obj_t *again_but=lv_button_create(screen1);
    lv_obj_set_size(again_but,200,80);
    lv_obj_set_style_bg_color(again_but, lv_color_hex(0x75FA61), 0);
    lv_obj_align(again_but, LV_ALIGN_RIGHT_MID,-100,0);
    lv_obj_t * label1 = lv_label_create(again_but);
    lv_label_set_text(label1, "Again");
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_24, 0);
    lv_obj_center(label1);
    
    lv_obj_t *label_2048=lv_label_create(screen1);
    lv_label_set_text(label_2048, "2048");
    lv_obj_set_style_text_font(label_2048, &lv_font_montserrat_30, 0);
    lv_obj_set_align(label_2048, LV_ALIGN_TOP_RIGHT);

    //先随机生成2个数
    spawn_random_tile();
    spawn_random_tile();
    update_grid();

    //给屏幕绑定滑动事件
    lv_obj_add_event_cb(screen1,scr_dir_event_cb,LV_EVENT_GESTURE,NULL);
    //重新开始按钮添加事件
    lv_obj_add_event_cb(again_but,again_but_event_cb,LV_EVENT_CLICKED,NULL);
    char * cmd ="mplayer -quiet -slave -loop 0 -input file=./my_fifo ./3.ogg &";
    system(cmd);
    
}

void go_man1_button_event_cb(lv_event_t *e)
{   
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        
        lv_screen_load(man_src);
        lv_obj_del(screen);
        
    }
    
}

//创建一个图片
void image()
{
    lv_obj_t *image=lv_image_create(screen);
    lv_obj_set_pos(image,0,0);
    lv_image_set_src(image,"A:./1.jpeg");//图片的地址
}
//登录界面
void into_screeen()
{
    //创建登录界面
    screen=lv_obj_create(NULL);
    lv_screen_load(screen);
    image();
    

    textarea1=lv_textarea_create(lv_screen_active());
    textarea2=lv_textarea_create(lv_screen_active());

    lv_obj_t *label_name=lv_label_create(screen);
    lv_obj_t *label_mima=lv_label_create(screen);
    
    lv_obj_set_size(label_name,100,50);
    lv_obj_set_pos(label_name,80,200);
    lv_obj_set_size(label_mima,100,50);
    lv_obj_set_pos(label_mima,80,300);
    
    lv_label_set_text(label_name,"name");
    lv_label_set_text(label_mima,"mima");
    lv_obj_set_style_text_font(label_name,&lv_font_montserrat_24,0);
    lv_obj_set_style_text_font(label_mima,&lv_font_montserrat_24,0);
    lv_obj_set_style_text_color(label_name, lv_color_hex(0x0000FF), 0);
    lv_obj_set_style_text_color(label_mima, lv_color_hex(0x0000FF), 0);

    lv_obj_set_size(textarea1,200,50);
    lv_obj_set_pos(textarea1,200,200);

    
    lv_obj_set_size(textarea2,200,50);
    lv_obj_set_pos(textarea2,200,300);
    //创建软键盘
    lv_obj_t *keyboard=lv_keyboard_create(lv_screen_active());
    //给软键盘添加一个隐藏标志
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);
    //登录按钮
    lv_obj_t *button=lv_button_create(lv_screen_active());
    lv_obj_set_size(button,150,50);
    lv_obj_set_align(button, LV_ALIGN_BOTTOM_MID);
    lv_obj_t * label = lv_label_create(button);
    lv_label_set_text(label, "next");
    lv_obj_center(label);
    //回到主界面按钮
    lv_obj_t *esc_button=lv_button_create(lv_screen_active());
    lv_obj_set_size(esc_button,150,50);
    lv_obj_set_align(esc_button, LV_ALIGN_OUT_LEFT_TOP);
    lv_obj_t * label_ESC = lv_label_create(esc_button);
    lv_label_set_text(label_ESC, "ESC");
    lv_obj_center(label_ESC);
    lv_obj_add_event_cb(esc_button,go_man1_button_event_cb,LV_EVENT_CLICKED,NULL);
    
    
    lv_obj_add_event_cb(textarea1,textarea_event_cb,LV_EVENT_FOCUSED,(void *)keyboard);
    lv_obj_add_event_cb(textarea1,textarea_event_cb,LV_EVENT_DEFOCUSED,(void *)keyboard);
    lv_obj_add_event_cb(textarea2,textarea_event_cb,LV_EVENT_FOCUSED,(void *)keyboard);
    lv_obj_add_event_cb(textarea2,textarea_event_cb,LV_EVENT_DEFOCUSED,(void *)keyboard);

    lv_obj_add_event_cb(button,button_event_cb,LV_EVENT_CLICKED,NULL);
    
    
}