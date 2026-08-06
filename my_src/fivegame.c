#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "fivegame.h"
#include <string.h>

extern lv_obj_t *man_src;
static lv_obj_t *five_screen=NULL;
static int board[GOMOKU_BOARD_SIZE+1][GOMOKU_BOARD_SIZE+1];//保存每个位置下棋的类型，0为空，1为黑，2为白
static int player;//初始棋手为黑,2为白
lv_obj_t *player_label=NULL;

int check_game_over(int last_x, int last_y) 
{
    // 定义需要检查的四个方向数组
    // 五子棋需要检查所有可能形成五连珠的方向
    //1:水平方向，2：垂直方向，3：主对角线，4：副对角线
    int directions[4] = {0, 1, 2, 3};
    
    // 循环检查四个方向
    // 每个方向都检查是否有连续5个相同颜色的棋子
    int WIN_COUNT=5;
    for (int d = 0; d < 4; d++) 
    {
        // 初始化计数器为1，因为当前落子位置已经有一个棋子
        // 从当前棋子开始向两边延伸计数
        int count = 1;
        
        // 定义方向向量变量，用于计算相邻位置坐标
        // dx和dy表示在每个方向上的增量（网格坐标增量为1）
        int dx = 0, dy = 0;
        
        // 根据当前检查的方向设置方向向量
        // switch语句根据方向枚举值设置不同的dx, dy值
        switch (directions[d]) {
            case 0: 
                // 水平方向：x坐标变化，y坐标不变
                dx = 1; 
                dy = 0; 
                break;
            case 1:   
                // 垂直方向：x坐标不变，y坐标变化
                dx = 0; 
                dy = 1; 
                break;
            case 2: 
                // 主对角线（向右下）：x和y坐标都增加
                dx = 1; 
                dy = 1; 
                break;
            case 3: 
                // 副对角线（向右上）：x坐标增加，y坐标减少
                dx = 1; 
                dy = -1; 
                break;
        }
        
        // 正向检查：从当前棋子向正方向延伸
        // step从1开始，因为当前棋子已经计数
        for (int step = 1; step < WIN_COUNT; step++) 
        {
            // 计算正方向上第step个位置的坐标
            // 使用当前坐标加上步长乘以方向向量
            int nx = last_x + step * dx;  // 计算新的网格x坐标
            int ny = last_y + step * dy;  // 计算新的网格y坐标
            
            // 检查新位置是否在棋盘范围内并且有相同颜色的棋子
            // 三个条件必须同时满足：
            // 1. nx在0到GOMOKU_BOARD_SIZE-1之间
            // 2. ny在0到GOMOKU_BOARD_SIZE-1之间
            // 3. board[nx][ny] == player 该位置棋子颜色与当前玩家相同
            if (nx >= 0 && nx < GOMOKU_BOARD_SIZE && ny >= 0 && ny < GOMOKU_BOARD_SIZE && board[nx][ny] == player) 
            {
                // 如果满足条件，计数器加1
                count++;
            } else 
            {
                // 如果不满足条件（出界或棋子不同），停止向这个方向检查
                break;
            }
        }
        
        // 反向检查：从当前棋子向反方向延伸
        // 原理与正向检查相同，只是方向相反
        for (int step = 1; step < WIN_COUNT; step++) 
        {
            // 计算反方向上第step个位置的坐标
            // 使用当前坐标减去步长乘以方向向量
            int nx = last_x - step * dx;  // 计算新的网格x坐标（反方向）
            int ny = last_y - step * dy;  // 计算新的网格y坐标（反方向）
            
            // 同样的边界和棋子颜色检查
            if (nx >= 0 && nx < GOMOKU_BOARD_SIZE && ny >= 0 && ny < GOMOKU_BOARD_SIZE && board[nx][ny] == player) 
            {
                // 计数器加1
                count++;
            } 
            else 
            {
                // 遇到不连续的棋子或边界，停止检查
                break;
            }
        }
        
        // 判断当前方向上是否满足获胜条件
        // count >= WIN_COUNT 表示有连续5个或更多相同颜色的棋子
        if (count >= WIN_COUNT) 
        {
            // 返回玩家编号作为胜利标志
            // 类型转换为int，因为函数返回int类型
            return player;
        }
    }
    
    // 如果没有玩家获胜，检查是否平局（棋盘已满）
    // 平局条件：棋盘上所有位置都有棋子
    
    // 初始化棋盘已满标志为真（1）
    // 假设棋盘是满的，如果找到空位则设置为假（0）
    int is_full = 1;
    
    // 遍历棋盘的每一个位置
    // 外层循环遍历每一列（x坐标）
    for (int i = 0; i < GOMOKU_BOARD_SIZE+1; i++) 
    {
        // 内层循环遍历每一行（y坐标）
        for (int j = 0; j < GOMOKU_BOARD_SIZE+1; j++) 
        {
            // 检查当前位置是否为空
            if (board[i][j] == 0) 
            {
                // 如果找到空位，设置标志为假
                is_full = 0;
                // 找到空位就可以提前退出内层循环
                break;
            }
        }
        // 如果已经找到空位，提前退出外层循环
        // 不需要继续检查剩余位置
        if (!is_full) break;
    }
    
    // 判断棋盘是否真的满了
    if (is_full) 
    {
        // 棋盘已满且没有玩家获胜，返回平局标志
        return -1;
    }
    
    // 如果没有获胜也没有平局，游戏继续
    return 0;
}

// 重置游戏函数
static void reset_game() 
{
    // 重置棋盘数组
    // 使用memset函数将整个棋盘数组填充为0，更高效的方式
    memset(board, 0, sizeof(board));  // string.h中定义的标准函数
    
    // 清除当前屏幕上的所有棋子
    // 先获取子对象总数
    uint32_t child_count = lv_obj_get_child_count(five_screen);
    
    // 遍历所有子对象（从后往前遍历，避免删除对象后索引变化导致漏删）
    for (int i = child_count - 1; i >= 0; i--) {
        lv_obj_t *child = lv_obj_get_child(five_screen, i);
        
        // 使用用户数据检测棋子对象
        if (lv_obj_get_user_data(child) && 
            strcmp((const char*)lv_obj_get_user_data(child), "gomoku_piece") == 0) {
            // 删除棋子对象
            lv_obj_del(child);
        }
    }
    
    // 重新初始化玩家状态
    player = 1; // 初始为黑棋
    lv_label_set_text(player_label, "Current: Black"); // 更新玩家标签
}

// 游戏结束消息框回调函数
static void game_over_msg_event_cb(lv_event_t *e) 
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *msg = (lv_obj_t *)lv_event_get_user_data(e);
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        reset_game();
        lv_msgbox_close(msg);
    }
}

void five_screen_event_cb(lv_event_t *e)
{
    lv_event_code_t code=lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) 
    {   // 如果是点击事件且游戏进行中
        lv_point_t touch_point;                      // 定义触摸点结构体
        lv_indev_get_point(lv_indev_get_act(), &touch_point); // 获取触摸点坐标
        
        // 检查触摸点是否在棋盘区域内
        // 棋盘在屏幕上的位置：水平居中，垂直距顶50像素
        int board_start_x = (1024 - GOMOKU_BOARD_WIDTH) / 2; // 棋盘起始X坐标
        int board_start_y = 50;                       // 棋盘起始Y坐标
        
        if (touch_point.x < board_start_x || touch_point.x > board_start_x + GOMOKU_BOARD_WIDTH || touch_point.y < board_start_y || touch_point.y > board_start_y + GOMOKU_BOARD_HEIGHT) 
        { 
                return ;                                   // 点击在棋盘外，直接返回
        }
        
        // 计算网格坐标（0-15）
        int grid_x = (touch_point.x - board_start_x) / GOMOKU_CELL_SIZE;
        int grid_y = (touch_point.y - board_start_y) / GOMOKU_CELL_SIZE;
        
        // 确保网格坐标在有效范围内
        if (grid_x < 0) grid_x = 0;
        if (grid_x >= GOMOKU_BOARD_SIZE) grid_x = GOMOKU_BOARD_SIZE ;
        if (grid_y < 0) grid_y = 0;
        if (grid_y >= GOMOKU_BOARD_SIZE) grid_y = GOMOKU_BOARD_SIZE ;
        
        // 检查该位置是否已有棋子
        if (board[grid_x][grid_y] != 0) 
        {
            return ;                                   // 位置已有棋子，直接返回
        }
        
        // 在棋盘数组中记录棋子
        board[grid_x][grid_y] = player;
        
        // 计算棋子在屏幕上的显示位置
        int piece_x = board_start_x + grid_x * GOMOKU_CELL_SIZE;
        int piece_y = board_start_y + grid_y * GOMOKU_CELL_SIZE;
        
        // 创建棋子对象
        lv_obj_t *piece = lv_obj_create(five_screen); // 在游戏屏幕上创建棋子
        lv_obj_set_size(piece, 30, 30);               // 设置棋子大小（略小于格子）
        lv_obj_set_pos(piece, (piece_x -15), (piece_y -15 )); // 设置棋子位置（居中）
        lv_obj_set_style_radius(piece, 15, 0);        // 设置棋子为圆形
        // 使用用户数据标记棋子对象
        lv_obj_set_user_data(piece, (void*)"gomoku_piece");
        
        // 根据当前玩家设置棋子颜色
        if (player == 1) 
        {                    // 黑棋
            lv_obj_set_style_bg_color(piece, lv_color_hex(0x000000), 0);
            lv_obj_set_style_border_width(piece, 0, 0); // 黑棋无边框
        } 
        else 
        {                                      // 白棋
            lv_obj_set_style_bg_color(piece, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_border_color(piece, lv_color_hex(0x000000), 0); // 白棋有黑色边框
            lv_obj_set_style_border_width(piece, 2, 0); // 设置边框宽度
        }
        // 检查游戏是否结束
        // 传递网格坐标而不是屏幕坐标
        int game_result = check_game_over(grid_x, grid_y);
        
        if (game_result != 0) 
        {                       // 游戏结束（胜利或平局）                        
            char msg_text[50];
            
            // 准备消息框文本
            if (game_result == 1) 
            {
                sprintf(msg_text, "Black Wins!"); // 黑棋胜利
            } 
            else if (game_result == 2) 
            {
                sprintf(msg_text, "White Wins!"); // 白棋胜利
            } 
            else if (game_result == -1) 
            {
                sprintf(msg_text, "Game Draw!");  // 平局
            }
            
            // 更新状态标签显示
            lv_label_set_text(player_label, msg_text);
            sleep(2);
            // 创建游戏结束消息框
            lv_obj_t *msg = lv_msgbox_create(five_screen);
            lv_msgbox_add_title(msg, "Game Over");
            lv_msgbox_add_text(msg, msg_text);
            
            // 添加确定按钮
            lv_obj_t *ok_button =lv_msgbox_add_footer_button(msg, "OK");
            
            // 为消息框按钮添加事件回调
            lv_obj_add_event_cb(ok_button, game_over_msg_event_cb, LV_EVENT_CLICKED, msg);
            
        } else 
        {   
            // 游戏继续
            // 切换玩家
            player = (player == 1) ? 2 : 1;
            
            // 更新玩家标签显示
            if (player == 1) 
            {
                lv_label_set_text(player_label, "Current: Black");
            } else 
            {
                lv_label_set_text(player_label, "Current: White");
            }
        }
    }
}

 
void draw_game_board(void) 
{
    // 计算棋盘在屏幕上的起始位置（居中显示）
    int board_start_x = (1024 - GOMOKU_BOARD_WIDTH) / 2; // 棋盘水平居中
    int board_start_y = 50;                       // 棋盘垂直距顶部50像素
    
    // 创建棋盘背景
    lv_obj_t *board_bg = lv_obj_create(five_screen); // 在游戏屏幕上创建背景对象
    lv_obj_remove_flag(board_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(board_bg, GOMOKU_BOARD_WIDTH + 20, GOMOKU_BOARD_HEIGHT + 20); // 设置背景大小（包含边框）
    lv_obj_set_pos(board_bg, board_start_x - 10, board_start_y - 10); // 设置背景位置
    lv_obj_set_style_bg_color(board_bg, lv_color_hex(0xFFE6B3), 0); // 设置背景颜色
    lv_obj_set_style_radius(board_bg, 5, 0);      // 设置圆角半径
    
    // 绘制网格线
    for (int i = 0; i <= GOMOKU_BOARD_SIZE; i++) 
    { 
        // 遍历所有网格线位置
        // 绘制垂直线
        lv_obj_t *v_line = lv_obj_create(five_screen); // 创建垂直线对象
        lv_obj_remove_flag(v_line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(v_line, 2, GOMOKU_BOARD_HEIGHT); // 设置垂直线大小
        lv_obj_set_pos(v_line, board_start_x + i * GOMOKU_CELL_SIZE - 1, board_start_y);  
        lv_obj_set_style_bg_color(v_line, lv_color_hex(0x000000), 0); // 设置线条颜色
        lv_obj_set_style_border_width(v_line, 0, 0); // 去掉边框
        
        // 绘制水平线
        lv_obj_t *h_line = lv_obj_create(five_screen); // 创建水平线对象
        lv_obj_remove_flag(h_line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(h_line, GOMOKU_BOARD_WIDTH, 2); // 设置水平线大小
        lv_obj_set_pos(h_line, board_start_x, board_start_y + i * GOMOKU_CELL_SIZE - 1); 
        lv_obj_set_style_bg_color(h_line, lv_color_hex(0x000000), 0); // 设置线条颜色
        lv_obj_set_style_border_width(h_line, 0, 0); // 去掉边框
    }
    
    // 绘制棋盘上的星位标记（标准五子棋有5个星位）
    int star_positions[5][2] = {                     // 星位坐标（相对于棋盘）
        {3, 3},   // 左上星位
        {3, 11},  // 右上星位
        {7, 7},   // 中心星位（天元）
        {11, 3},  // 左下星位
        {11, 11}  // 右下星位
    };
    
    for (int i = 0; i < 5; i++) 
    {                    
        // 遍历5个星位
        int star_x = board_start_x + star_positions[i][0] * GOMOKU_CELL_SIZE; // 计算星位X坐标
        int star_y = board_start_y + star_positions[i][1] * GOMOKU_CELL_SIZE; // 计算星位Y坐标
        
        lv_obj_t *star = lv_obj_create(five_screen); // 创建星位对象
        lv_obj_remove_flag(star, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(star, 6, 6);                 // 设置星位大小
        lv_obj_set_pos(star, star_x - 3, star_y - 3); // 设置星位位置（居中）
        lv_obj_set_style_bg_color(star, lv_color_hex(0x000000), 0); // 设置星位颜色
        lv_obj_set_style_radius(star, 3, 0);         // 设置星位为圆形
        lv_obj_set_style_border_width(star, 0, 0);   // 去掉边框
    }
}



// 重新开始按钮回调函数
static void restart_game_cb(lv_event_t *e) 
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    { 
        reset_game();
    }
}

// 回到主菜单按钮回调函数
void go_man2_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        
        lv_screen_load(man_src);
        lv_obj_del(five_screen);
    }
    
}

void fivegame_init()
{
    five_screen=lv_obj_create(NULL);
    lv_screen_load(five_screen);
    
    // 回到主菜单按钮
    lv_obj_t *esc_button=lv_button_create(five_screen);
    lv_obj_set_size(esc_button,150,50);
    lv_obj_set_align(esc_button, LV_ALIGN_TOP_RIGHT);
    lv_obj_t * label_ESC = lv_label_create(esc_button);
    lv_label_set_text(label_ESC, "ESC");
    lv_obj_center(label_ESC);
    lv_obj_add_event_cb(esc_button,go_man2_button_event_cb,LV_EVENT_CLICKED,NULL);
    
    // 重新开始游戏按钮
    lv_obj_t *restart_btn=lv_button_create(five_screen);
    lv_obj_set_size(restart_btn,150,50);
    lv_obj_set_align(restart_btn, LV_ALIGN_TOP_LEFT);
    lv_obj_set_pos(restart_btn, 20, 60); // 设置按钮位置，避免与玩家标签重叠
    lv_obj_t *label_restart = lv_label_create(restart_btn);
    lv_label_set_text(label_restart, "Restart");
    lv_obj_center(label_restart);
    lv_obj_add_event_cb(restart_btn,restart_game_cb,LV_EVENT_CLICKED,NULL);

    player=1;//初始为黑
    player_label = lv_label_create(five_screen);     // 创建玩家标签
    lv_label_set_text(player_label, "Current: Black"); // 设置初始文本
    lv_obj_set_style_text_color(player_label, lv_color_hex(0x000000), 0); // 设置文字颜色
    lv_obj_set_style_text_font(player_label, &lv_font_montserrat_20, 0); // 设置字体大小
    lv_obj_align(player_label, LV_ALIGN_TOP_LEFT, 20, 10); // 对齐到左上角

    draw_game_board();
    
    lv_obj_add_event_cb(five_screen,five_screen_event_cb,LV_EVENT_CLICKED,NULL);
}