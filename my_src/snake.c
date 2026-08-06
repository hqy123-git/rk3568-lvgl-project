#include <stdio.h>
#include "snake.h"
 
// 游戏结构体变量，各个成员都有重要意义，定义为全局，方便访问
static SnakeGame game;
lv_obj_t *srceen=NULL;
extern lv_obj_t *man_src;

// 初始化蛇
static void init_snake() {
    // 清除之前的蛇身
    for(int i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++) {
        if(game.snake_parts[i]) {
            lv_obj_del(game.snake_parts[i]);
            game.snake_parts[i] = NULL;
        }
    }
    
    // 初始化蛇的位置（在屏幕中央）
    game.snake_length = 3;
    int start_x = GRID_WIDTH / 2;
    int start_y = GRID_HEIGHT / 2;
    
    for(int i = 0; i < game.snake_length; i++) {
        game.snake_pos[i].x = start_x - i;
        game.snake_pos[i].y = start_y;
        
        // 创建蛇身显示对象
        game.snake_parts[i] = lv_obj_create(game.game_area);
        lv_obj_set_size(game.snake_parts[i], GRID_SIZE - 2, GRID_SIZE - 2);
        lv_obj_set_pos(game.snake_parts[i], 
                      game.snake_pos[i].x * GRID_SIZE + 1, 
                      game.snake_pos[i].y * GRID_SIZE + 1);
        
        // 蛇头颜色不同
        if(i == 0) {
            lv_obj_set_style_bg_color(game.snake_parts[i], lv_color_hex(0xEF88BE), 0);
        } else {
            lv_obj_set_style_bg_color(game.snake_parts[i], lv_color_hex(0x8BC34A), 0);
        }
        
        lv_obj_set_style_radius(game.snake_parts[i], 5, 0);
        lv_obj_set_style_border_width(game.snake_parts[i], 0, 0);
    }
    
    // 初始化方向
    game.current_dir = DIR_RIGHT;
    game.next_dir = DIR_RIGHT;
}


// 生成食物
static void generate_food() {
    // 移除旧食物
    if(game.food_obj) {
        lv_obj_del(game.food_obj);
    }
    
    // 随机生成食物位置，确保不在蛇身上
    int valid_position = 0;
    while(!valid_position) {
        valid_position = 1;
        game.food_pos.x = rand() % GRID_WIDTH;
        game.food_pos.y = rand() % GRID_HEIGHT;
        
        // 检查是否与蛇身重叠
        for(int i = 0; i < game.snake_length; i++) {
            if(game.snake_pos[i].x == game.food_pos.x && 
               game.snake_pos[i].y == game.food_pos.y) {
                valid_position = 0;
                break;
            }
        }
    }
    
    // 创建食物显示对象
    game.food_obj = lv_obj_create(game.game_area);
    lv_obj_set_size(game.food_obj, GRID_SIZE - 2, GRID_SIZE - 2);
    lv_obj_set_pos(game.food_obj, 
                  game.food_pos.x * GRID_SIZE + 1, 
                  game.food_pos.y * GRID_SIZE + 1);
    lv_obj_set_style_bg_color(game.food_obj, lv_color_hex(0xFF5252), 0);
    lv_obj_set_style_radius(game.food_obj, 5, 0);
    lv_obj_set_style_border_width(game.food_obj, 0, 0);
}
// 游戏结束
static void game_over() {
    game.state = SNAKE_GAME_OVER;
    lv_label_set_text(game.state_label, "Game stat:game over");
    lv_obj_set_style_text_color(game.state_label, lv_color_hex(0xFF5252), 0);
    
    // 停止游戏定时器
    if(game.game_timer) {
        lv_timer_pause(game.game_timer);
    }
    lv_obj_t *msg=lv_msgbox_create(srceen);
    lv_msgbox_add_close_button(msg);
    lv_msgbox_add_title(msg,"Hint");
    
    
    lv_msgbox_add_text(msg,"game over!");
    
}

// 开始游戏
static void start_game() 
{
    if(game.state == SNAKE_GAME_START)
    {
        return ;
    }
    if(game.state == SNAKE_GAME_OVER) {
        // 重置游戏
        game.score = 0;
        lv_label_set_text_fmt(game.score_label, "Score:%d", game.score);
        lv_label_set_text(game.state_label, "Game stat:run game");
        lv_obj_set_style_text_color(game.state_label, lv_color_hex(0x4CAF50), 0);
        game.period = 250;
        lv_timer_set_period(game.game_timer,game.period);
        // 重新初始化蛇和食物
        init_snake();
        generate_food();
    }
    
    game.state = SNAKE_GAME_START;
    lv_label_set_text(game.state_label, "Game stat:run game");
    lv_obj_set_style_text_color(game.state_label, lv_color_hex(0x4CAF50), 0);
    
    // 启动游戏定时器
    if(game.game_timer) {
        lv_timer_resume(game.game_timer);
    }
}

// 暂停游戏
static void pause_game() {
    if(game.state == SNAKE_GAME_START) {
        game.state = SNAKE_GAME_PAUSED;
        lv_label_set_text(game.state_label, "Game stat:Paused");
        lv_obj_set_style_text_color(game.state_label, lv_color_hex(0xFF9800), 0);
        
        if(game.game_timer) {
            lv_timer_pause(game.game_timer);
        }
    } else if(game.state == SNAKE_GAME_PAUSED) {
        game.state = SNAKE_GAME_START;
        lv_label_set_text(game.state_label, "Game stat:run game");
        lv_obj_set_style_text_color(game.state_label, lv_color_hex(0x4CAF50), 0);
        
        if(game.game_timer) {
            lv_timer_resume(game.game_timer);
        }
    }
}

// 重新开始游戏
static void restart_game() {
    game.score = 0;
    lv_label_set_text_fmt(game.score_label, "Score:%d", game.score);
    
    init_snake();
    generate_food();
    
    game.state = SNAKE_GAME_START;
    lv_label_set_text(game.state_label, "Game stat:run game");
    lv_obj_set_style_text_color(game.state_label, lv_color_hex(0x4CAF50), 0);
    
    if(game.game_timer) {
        game.period = 250;
        lv_timer_set_period(game.game_timer,game.period);
        lv_timer_resume(game.game_timer);
    }
}


// 按钮回调函数
static void start_game_cb(lv_event_t *e) {
    start_game();
}

static void pause_game_cb(lv_event_t *e) {
    pause_game();
}

static void restart_game_cb(lv_event_t *e) {
    restart_game();
}

// 创建游戏区域网格(该区域起始位置 (0,0) 大小(700,600))
static void create_game_area(lv_obj_t *parent) {
    // 创建游戏区域容器
    game.game_area = lv_obj_create(parent);
    lv_obj_set_size(game.game_area, SNAKE_GAME_AREA_WIDTH, SNAKE_GAME_AREA_HEIGHT);
    lv_obj_set_pos(game.game_area, 0, 0);
    lv_obj_set_style_border_width(game.game_area, 0, 0);
    lv_obj_set_style_pad_all(game.game_area,0,0);
    lv_obj_set_style_border_color(game.game_area, lv_color_hex(0x4A90E2), 0);
    lv_obj_set_style_bg_color(game.game_area, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_radius(game.game_area, 0, 0);
    lv_obj_clear_flag(game.game_area, LV_OBJ_FLAG_SCROLLABLE);//清除游戏区域滚动效果
    
    // 绘制网格线
    //绘制竖线
    for(int i = 0; i <= GRID_WIDTH; i++) {
        lv_obj_t *line = lv_obj_create(game.game_area);
        lv_obj_set_size(line, 1, SNAKE_GAME_AREA_HEIGHT);
        lv_obj_set_pos(line, i * GRID_SIZE, 0);
        lv_obj_set_style_bg_color(line, lv_color_hex(0x2E2E4E), 0);
        lv_obj_set_style_border_width(line, 0, 0);
    }
    //绘制横线
    for(int i = 0; i <= GRID_HEIGHT; i++) {
        lv_obj_t *line = lv_obj_create(game.game_area);
        lv_obj_set_size(line, SNAKE_GAME_AREA_WIDTH, 1);
        lv_obj_set_pos(line, 0, i * GRID_SIZE);
        lv_obj_set_style_bg_color(line, lv_color_hex(0x2E2E4E), 0);
        lv_obj_set_style_border_width(line, 0, 0);
    }
}

// 创建控制面板(该区域起始位置 (710,0) 大小(300,600))
static void create_control_panel(lv_obj_t *parent) 
{
    // 游戏标题
    lv_obj_t *title = lv_label_create(parent);
    lv_obj_set_pos(title,710,20);
    lv_obj_set_size(title,300,60);
    lv_obj_set_style_text_font(title,&lv_font_montserrat_24, 0);
    lv_label_set_text(title, "snake game");
    lv_obj_set_style_text_color(title, lv_color_hex(0x4FC3F7), 0);

    
    // 显示当前分数的标签
    game.score_label = lv_label_create(parent);
    lv_obj_set_pos(game.score_label,710,90);
    lv_obj_set_size(game.score_label,300,60);
    lv_label_set_text(game.score_label, "Score:0");
    lv_obj_set_style_text_color(game.score_label, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_text_font(game.score_label, &lv_font_montserrat_24, 0);
    
    // 显示最高分的标签
    game.high_score_label = lv_label_create(parent);
    lv_obj_set_pos(game.high_score_label,710,160);
    lv_obj_set_size(game.high_score_label,300,60);
    lv_label_set_text(game.high_score_label, "max_score:0");
    lv_obj_set_style_text_color(game.high_score_label, lv_color_hex(0xFF9800), 0);
    lv_obj_set_style_text_font(game.high_score_label, &lv_font_montserrat_24, 0);
    
    // 显示游戏状态的标签
    game.state_label = lv_label_create(parent);
    lv_obj_set_pos(game.state_label,710,230);
    lv_obj_set_size(game.state_label,300,60);
    lv_label_set_text(game.state_label, "Game State:loading");
    lv_obj_set_style_text_color(game.state_label, lv_color_hex(0x4FC3F7), 0);
    lv_obj_set_style_text_font(game.state_label, &lv_font_montserrat_24, 0);
    
    // 开始按钮
    lv_obj_t *start_btn = lv_btn_create(parent);
    lv_obj_set_size(start_btn, 300, 60);
    lv_obj_set_pos(start_btn,710,300);
    lv_obj_add_event_cb(start_btn, start_game_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *start_label = lv_label_create(start_btn);
    lv_obj_set_style_text_font(start_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(start_label, "run game");
    lv_obj_center(start_label);
    
    // 暂停/继续按钮
    lv_obj_t *pause_btn = lv_btn_create(parent);
    lv_obj_set_size(pause_btn, 300, 60);
    lv_obj_set_pos(pause_btn,710,370);
    lv_obj_add_event_cb(pause_btn, pause_game_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *pause_label = lv_label_create(pause_btn);
    lv_obj_set_style_text_font(pause_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(pause_label, "Paused/Go on");
    lv_obj_center(pause_label);
    
    // 重新开始按钮
    lv_obj_t *restart_btn = lv_btn_create(parent);
    lv_obj_set_size(restart_btn, 300, 60);
    lv_obj_set_pos(restart_btn,710,440);
    lv_obj_add_event_cb(restart_btn, restart_game_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *restart_label = lv_label_create(restart_btn);
    lv_obj_set_style_text_font(restart_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(restart_label, "Restart");
    lv_obj_center(restart_label);
    
}


// 游戏定时器回调
static void game_timer_cb(lv_timer_t *timer) {
    if(game.state != SNAKE_GAME_START) return;

    // 更新方向
    game.current_dir = game.next_dir;
    
    // 保存蛇尾位置（用于可能增加长度）
    lv_point_t prev_tail = game.snake_pos[game.snake_length - 1];
    
    // 移动蛇身（从尾部向前移动）
    for(int i = game.snake_length - 1; i > 0; i--) {
        game.snake_pos[i] = game.snake_pos[i - 1];
    }
    
    // 移动蛇头
    switch(game.current_dir) {
        case DIR_UP:
            game.snake_pos[0].y--;
            break;
        case DIR_DOWN:
            game.snake_pos[0].y++;
            break;
        case DIR_LEFT:
            game.snake_pos[0].x--;
            break;
        case DIR_RIGHT:
            game.snake_pos[0].x++;
            break;
    }
    
    // 检查是否撞墙
    if(game.snake_pos[0].x < 0 || game.snake_pos[0].x >= GRID_WIDTH ||
       game.snake_pos[0].y < 0 || game.snake_pos[0].y >= GRID_HEIGHT) {
        game_over();
        return;
    }
    
    // 检查是否撞到自己
    for(int i = 1; i < game.snake_length; i++) {
        if(game.snake_pos[0].x == game.snake_pos[i].x &&
           game.snake_pos[0].y == game.snake_pos[i].y) {
            game_over();
            return;
        }
    }
    
    // 检查是否吃到食物
    if(game.snake_pos[0].x == game.food_pos.x &&game.snake_pos[0].y == game.food_pos.y) 
       {
            // 增加蛇长度
            game.snake_pos[game.snake_length] = prev_tail;
            
            // 创建新的蛇身段
            game.snake_parts[game.snake_length] = lv_obj_create(game.game_area);
            lv_obj_set_size(game.snake_parts[game.snake_length], GRID_SIZE - 2, GRID_SIZE - 2);
            lv_obj_set_pos(game.snake_parts[game.snake_length], 
                        prev_tail.x * GRID_SIZE + 1, 
                        prev_tail.y * GRID_SIZE + 1);
            lv_obj_set_style_bg_color(game.snake_parts[game.snake_length], lv_color_hex(0x8BC34A), 0);
            lv_obj_set_style_radius(game.snake_parts[game.snake_length], 5, 0);
            lv_obj_set_style_border_width(game.snake_parts[game.snake_length], 0, 0);
            
            game.snake_length++;
            //根据蛇的长度改变蛇运动一步的时间，每增长1个单位，就减少3ms
            game.period -= 3;
            lv_timer_set_period(game.game_timer,game.period);    

            // 增加分数
            game.score += 10;
            lv_label_set_text_fmt(game.score_label, "Score:%d", game.score);
            
            // 更新最高分
            if(game.score > game.high_score) 
            {
                game.high_score = game.score;
                lv_label_set_text_fmt(game.high_score_label, "max_score:%d", game.high_score);
            }
        
            // 生成新食物
            generate_food();
        }
    
    // 更新蛇的显示位置
    for(int i = 0; i < game.snake_length; i++) {
        if(game.snake_parts[i]) {
            lv_obj_set_pos(game.snake_parts[i], 
                          game.snake_pos[i].x * GRID_SIZE + 1, 
                          game.snake_pos[i].y * GRID_SIZE + 1);
        }
    }
}
// 触摸屏滑动事件(手势事件)处理
static void gestur_event_handler(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    // 只在拖动结束时获取手势方向
    if (code == LV_EVENT_GESTURE) 
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) 
        {
            case LV_DIR_TOP://上
                if(game.current_dir != DIR_DOWN) //不能原地掉头
                {
                    game.next_dir = DIR_UP;
                }
                break;
            case LV_DIR_BOTTOM://下
                if(game.current_dir != DIR_UP) 
                {
                    game.next_dir = DIR_DOWN;
                }
                break;
            case LV_DIR_LEFT://左
                if(game.current_dir != DIR_RIGHT) 
                {
                    game.next_dir = DIR_LEFT;
                }
                break;
            case LV_DIR_RIGHT://右
                if(game.current_dir != DIR_LEFT) 
                {
                    game.next_dir = DIR_RIGHT;
                }
                break;
            default:
                break;
        }
    }
    
}

void go_man_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) 
    {
        game.period = 250;
        lv_timer_delete(game.game_timer);
        lv_screen_load(man_src);
        lv_obj_del(srceen);
    }
}

// 初始化游戏
void snake_game_init() 
{
    // 初始化随机数种子
    srand(time(NULL));
    //创建游戏界面
    srceen=lv_obj_create(NULL);
    lv_screen_load(srceen);
    //回到主界面按钮
    lv_obj_t *esc_button=lv_button_create(lv_screen_active());
    lv_obj_set_size(esc_button, 300, 60);
    lv_obj_set_pos(esc_button,710,510);
    lv_obj_t * label_ESC = lv_label_create(esc_button);
    lv_label_set_text(label_ESC, "ESC");
    lv_obj_set_style_text_font(label_ESC,&lv_font_montserrat_24, 0);
    lv_obj_center(label_ESC);
    lv_obj_add_event_cb(esc_button,go_man_button_event_cb,LV_EVENT_CLICKED,NULL);
    // 初始化游戏状态
    game.state = SNAKE_GAME_READY;
    game.score = 0;
    game.high_score = 0;
    
    // 之前的项目把ui全部放在一起，函数太长了，故分为两部分
    // 创建游戏区域
    create_game_area(lv_screen_active());
    
    // 创建控制面板
    create_control_panel(lv_screen_active());
    
    // 初始化蛇和食物
    init_snake();
    generate_food();
    
    // 创建游戏定时器
    game.period = 250;
    game.game_timer = lv_timer_create(game_timer_cb, game.period, NULL);
    
    // 设置手势事件回调函数
    lv_obj_add_event_cb(lv_screen_active(),gestur_event_handler,LV_EVENT_GESTURE,NULL);
}