#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "thunder.h"
#include <string.h>

lv_obj_t *man_src;
static ThunderGame game;

// 初始化玩家函数
// 创建并初始化玩家飞机的所有属性和显示对象
static void init_player() {
    // 设置玩家飞机的初始位置（游戏容器底部中央）
    game.player.obj.x = THUNDER_GAME_AREA_WIDTH / 2 - PLAYER_WIDTH / 2;
    game.player.obj.y = THUNDER_GAME_AREA_HEIGHT - PLAYER_HEIGHT - 20;
    
    // 设置玩家飞机的尺寸
    game.player.obj.width = PLAYER_WIDTH;
    game.player.obj.height = PLAYER_HEIGHT;
    
    // 设置玩家飞机的移动速度
    game.player.obj.speed = PLAYER_SPEED;
    
    // 设置玩家飞机为活动状态
    game.player.obj.active = 1;
    
    // 创建玩家飞机的LVGL对象
    game.player.obj.obj = lv_obj_create(game.game_container);
    
    // 设置LVGL对象的尺寸
    lv_obj_set_size(game.player.obj.obj, PLAYER_WIDTH, PLAYER_HEIGHT);
    
    // 设置LVGL对象的位置
    lv_obj_set_pos(game.player.obj.obj, game.player.obj.x, game.player.obj.y);
    
    // 设置玩家飞机的颜色（蓝色）
    lv_obj_set_style_bg_color(game.player.obj.obj, lv_color_hex(0x00BFFF), 0);
    
    // 设置玩家飞机为圆形
    lv_obj_set_style_radius(game.player.obj.obj, 20, 0);
    
    // 初始化玩家分数
    game.player.score = 0;
    
    // 初始化玩家生命值
    game.player.lives = 3;
}

// 初始化子弹函数
// 创建并初始化所有子弹对象（对象池）
static void init_bullets() {
    // 遍历所有子弹槽位
    for (int i = 0; i < MAX_BULLETS; i++) {
        // 初始状态为非活动
        game.bullets[i].active = 0;
        
        // 设置子弹尺寸
        game.bullets[i].width = BULLET_WIDTH;
        game.bullets[i].height = BULLET_HEIGHT;
        
        // 设置子弹速度
        game.bullets[i].speed = BULLET_SPEED;
        
        // 创建子弹的LVGL对象
        game.bullets[i].obj = lv_obj_create(game.game_container);
        
        // 设置LVGL对象的尺寸
        lv_obj_set_size(game.bullets[i].obj, BULLET_WIDTH, BULLET_HEIGHT);
        
        // 设置子弹颜色（黄色）
        lv_obj_set_style_bg_color(game.bullets[i].obj, lv_color_hex(0xFFFF00), 0);
        
        // 设置子弹为圆角矩形
        lv_obj_set_style_radius(game.bullets[i].obj, 2, 0);
        
        // 默认隐藏子弹
        lv_obj_add_flag(game.bullets[i].obj, LV_OBJ_FLAG_HIDDEN);
    }
}

// 初始化敌人函数
// 创建并初始化所有敌人对象（对象池）
static void init_enemies() {
    // 遍历所有敌人槽位
    for (int i = 0; i < MAX_ENEMIES; i++) {
        // 初始状态为非活动
        game.enemies[i].active = 0;
        
        // 设置敌人尺寸
        game.enemies[i].width = ENEMY_WIDTH;
        game.enemies[i].height = ENEMY_HEIGHT;
        
        // 设置敌人速度
        game.enemies[i].speed = ENEMY_SPEED;
        
        // 创建敌人的LVGL对象
        game.enemies[i].obj = lv_obj_create(game.game_container);
        
        // 设置LVGL对象的尺寸
        lv_obj_set_size(game.enemies[i].obj, ENEMY_WIDTH, ENEMY_HEIGHT);
        
        // 设置敌人颜色（红色）
        lv_obj_set_style_bg_color(game.enemies[i].obj, lv_color_hex(0xFF4500), 0);
        
        // 设置敌人为圆形
        lv_obj_set_style_radius(game.enemies[i].obj, 15, 0);
        
        // 默认隐藏敌人
        lv_obj_add_flag(game.enemies[i].obj, LV_OBJ_FLAG_HIDDEN);
    }
}

// 发射子弹函数
// 从玩家飞机位置发射一枚子弹
static void fire_bullet() {
    // 遍历所有子弹，寻找非活动状态的子弹
    for (int i = 0; i < MAX_BULLETS; i++) {
        // 找到一个非活动的子弹
        if (!game.bullets[i].active) {
            // 计算子弹起始位置（从玩家飞机中心发射）
            game.bullets[i].x = game.player.obj.x + PLAYER_WIDTH / 2 - BULLET_WIDTH / 2;
            game.bullets[i].y = game.player.obj.y;
            
            // 设置子弹为活动状态
            game.bullets[i].active = 1;
            
            // 显示子弹（移除隐藏标志）
            lv_obj_remove_flag(game.bullets[i].obj, LV_OBJ_FLAG_HIDDEN);
            
            // 设置子弹在LVGL中的位置
            lv_obj_set_pos(game.bullets[i].obj, game.bullets[i].x, game.bullets[i].y);
            
            // 找到子弹后退出循环
            break;
        }
    }
}

// 生成敌人函数
// 在游戏容器顶部随机位置生成一个敌人
static void spawn_enemy() {
    // 遍历所有敌人，寻找非活动状态的敌人
    for (int i = 0; i < MAX_ENEMIES; i++) {
        // 找到一个非活动的敌人
        if (!game.enemies[i].active) {
            // 在游戏容器宽度范围内随机生成敌人的X坐标
            game.enemies[i].x = rand() % (THUNDER_GAME_AREA_WIDTH - ENEMY_WIDTH);
            // 设置敌人的Y坐标为游戏容器外上方（从顶部进入）
            game.enemies[i].y = -ENEMY_HEIGHT;
            
            // 设置敌人为活动状态
            game.enemies[i].active = 1;
            
            // 显示敌人（移除隐藏标志）
            lv_obj_remove_flag(game.enemies[i].obj, LV_OBJ_FLAG_HIDDEN);
            
            // 设置敌人在LVGL中的位置
            lv_obj_set_pos(game.enemies[i].obj, game.enemies[i].x, game.enemies[i].y);
            
            // 找到敌人后退出循环
            break;
        }
    }
}

// 检查碰撞函数
// 检查两个游戏对象是否发生碰撞
// 参数: a - 第一个游戏对象
//       b - 第二个游戏对象
// 返回值: 1表示碰撞，0表示未碰撞
static int check_collision(GameObject *a, GameObject *b) {
    // 碰撞检测算法（AABB - 轴对齐包围盒算法）
    // 当两个矩形的边界重叠时发生碰撞
    return (a->x < b->x + b->width &&        // a的左边界在b的右边界左侧
            a->x + a->width > b->x &&        // a的右边界在b的左边界右侧
            a->y < b->y + b->height &&       // a的上边界在b的下边界上方
            a->y + a->height > b->y);        // a的下边界在b的上边界下方
}

// 更新游戏函数
// 游戏的核心循环函数，负责更新所有游戏元素、检测碰撞和管理游戏状态
static void update_game() {
    // 更新所有活动的子弹
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullets[i].active) {
            // 更新子弹位置（向上移动）
            game.bullets[i].y -= game.bullets[i].speed;
            
            // 设置子弹在LVGL中的新位置
            lv_obj_set_pos(game.bullets[i].obj, game.bullets[i].x, game.bullets[i].y);
            
            // 检查子弹是否超出屏幕顶部
            if (game.bullets[i].y < -BULLET_HEIGHT) {
                game.bullets[i].active = 0;  // 设置为非活动状态
                lv_obj_add_flag(game.bullets[i].obj, LV_OBJ_FLAG_HIDDEN);  // 隐藏子弹
            }
        }
    }
    
    // 更新所有活动的敌人
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game.enemies[i].active) {
            // 更新敌人位置（向下移动）
            game.enemies[i].y += game.enemies[i].speed;
            
            // 设置敌人在LVGL中的新位置
            lv_obj_set_pos(game.enemies[i].obj, game.enemies[i].x, game.enemies[i].y);
            
            // 检查敌人是否超出游戏容器底部
            if (game.enemies[i].y > THUNDER_GAME_AREA_HEIGHT) {
                game.enemies[i].active = 0;  // 设置为非活动状态
                lv_obj_add_flag(game.enemies[i].obj, LV_OBJ_FLAG_HIDDEN);  // 隐藏敌人
            }
        }
    }
    
    // 检查子弹和敌人的碰撞
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (game.bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (game.enemies[j].active) {
                    // 检查碰撞
                    if (check_collision(&game.bullets[i], &game.enemies[j])) {
                        // 子弹击中敌人，处理碰撞结果
                        game.bullets[i].active = 0;  // 子弹消失
                        lv_obj_add_flag(game.bullets[i].obj, LV_OBJ_FLAG_HIDDEN);
                        
                        game.enemies[j].active = 0;  // 敌人消失
                        lv_obj_add_flag(game.enemies[j].obj, LV_OBJ_FLAG_HIDDEN);
                        
                        game.player.score += 10;  // 增加分数
                        lv_label_set_text_fmt(game.score_label, "Score: %d", game.player.score);  // 更新分数显示
                    }
                }
            }
        }
    }
    
    // 检查敌人和玩家的碰撞
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game.enemies[i].active) {
            // 检查碰撞
            if (check_collision(&game.enemies[i], &game.player.obj)) {
                // 敌人撞击玩家，处理碰撞结果
                game.enemies[i].active = 0;  // 敌人消失
                lv_obj_add_flag(game.enemies[i].obj, LV_OBJ_FLAG_HIDDEN);
                
                game.player.lives--;  // 减少生命值
                lv_label_set_text_fmt(game.lives_label, "Lives: %d", game.player.lives);  // 更新生命值显示
                
                // 检查游戏是否结束
                if (game.player.lives <= 0) {
                    game.game_running = 0;  // 设置游戏状态为停止
                    lv_timer_pause(game.game_timer);  // 暂停游戏定时器
                    
                    // 创建游戏结束消息框
                    lv_obj_t *msg = lv_msgbox_create(game.screen);
                    lv_msgbox_add_title(msg, "Game Over");  // 设置消息框标题
                    lv_msgbox_add_text(msg, "You lose! Game over.");  // 设置消息框内容
                    lv_msgbox_add_close_button(msg);  // 添加关闭按钮
                }
            }
        }
    }
    
    // 随机生成敌人（5%的概率）
    if (rand() % 100 < 5) {
        spawn_enemy();
    }
}

// 自动射击定时器回调函数
static void auto_fire_timer_cb(lv_timer_t *timer) {
    // 检查游戏是否正在运行且自动射击已启用
    if (!game.game_running || !game.auto_fire_enabled) return;
    
    // 自动发射子弹
    fire_bullet();
}

// 游戏定时器回调函数
// 每隔一段时间调用一次，更新游戏状态
// 参数: timer - LVGL定时器对象
static void game_timer_cb(lv_timer_t *timer) {
    // 检查游戏是否正在运行
    if (!game.game_running) return;
    
    // 调用更新游戏函数
    update_game();
}

// 回到主菜单函数
// 处理返回主菜单按钮的点击事件
// 参数: e - LVGL事件对象，包含事件类型和相关信息
void go_back_from_thunder(lv_event_t *e) {
    // 获取事件类型
    lv_event_code_t code = lv_event_get_code(e);
    
    // 处理点击事件
    if (code == LV_EVENT_CLICKED) {
        // 设置游戏状态为停止
        game.game_running = 0;
        
        // 删除游戏定时器（如果存在）
        if (game.game_timer) {
            lv_timer_delete(game.game_timer);
        }
        
        // 加载主菜单屏幕
        lv_screen_load(man_src);
        lv_obj_del(game.screen);
    }
}

// 左移按钮事件回调函数
static void left_button_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        game.player.obj.x -= PLAYER_SPEED;
        
        // 边界检查 - 确保飞机不超出游戏容器左边界
        if (game.player.obj.x < 0) game.player.obj.x = 0;
        
        // 更新玩家飞机在LVGL中的实际位置
        lv_obj_set_pos(game.player.obj.obj, game.player.obj.x, game.player.obj.y);
    }
}

// 右移按钮事件回调函数
static void right_button_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        game.player.obj.x += PLAYER_SPEED;
        
        // 边界检查 - 确保飞机不超出游戏容器右边界
        if (game.player.obj.x > THUNDER_GAME_AREA_WIDTH - PLAYER_WIDTH) 
            game.player.obj.x = THUNDER_GAME_AREA_WIDTH - PLAYER_WIDTH;
        
        // 更新玩家飞机在LVGL中的实际位置
        lv_obj_set_pos(game.player.obj.obj, game.player.obj.x, game.player.obj.y);
    }
}

// 发射按钮事件回调函数（切换自动射击）
static void fire_button_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 切换自动射击状态
        game.auto_fire_enabled = !game.auto_fire_enabled;
        
        // 更新按钮标签
        lv_obj_t *label = lv_obj_get_child(game.fire_button, NULL);
        if (game.auto_fire_enabled) {
            lv_label_set_text(label, "STOP");
            lv_timer_resume(game.auto_fire_timer);  // 启动自动射击定时器
        } else {
            lv_label_set_text(label, "AUTO");
            lv_timer_pause(game.auto_fire_timer);  // 暂停自动射击定时器
        }
    }
}

// 玩家控制事件回调函数
// 处理游戏中的用户交互事件，包括发射子弹和移动玩家飞机
// 参数: e - LVGL事件对象，包含事件类型和相关信息
void thunder_game_event_cb(lv_event_t *e) {
    // 获取事件类型
    lv_event_code_t code = lv_event_get_code(e);
    
    // 处理点击事件 - 发射子弹
    if (code == LV_EVENT_CLICKED) 
    {
        fire_bullet();  // 调用发射子弹函数
    }
    
    // 处理手势事件 - 移动玩家飞机
    if (code == LV_EVENT_GESTURE) 
    {   
        // 获取手势方向
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        
        // 根据手势方向移动玩家飞机
        switch(dir) {
            case LV_DIR_LEFT:  // 向左滑动
                game.player.obj.x -= PLAYER_SPEED;
                break;
            case LV_DIR_RIGHT: // 向右滑动
                game.player.obj.x += PLAYER_SPEED;
                break;
            default:
                break;
        }
        
        // 边界检查 - 确保飞机不超出游戏容器左边界
        if (game.player.obj.x < 0) game.player.obj.x = 0;
        
        // 边界检查 - 确保飞机不超出游戏容器右边界
        if (game.player.obj.x > THUNDER_GAME_AREA_WIDTH - PLAYER_WIDTH) 
            game.player.obj.x = THUNDER_GAME_AREA_WIDTH - PLAYER_WIDTH;
        
        // 更新玩家飞机在LVGL中的实际位置
        lv_obj_set_pos(game.player.obj.obj, game.player.obj.x, game.player.obj.y);
    }
}

// 初始化雷霆战机游戏
// 负责创建游戏界面、初始化游戏元素和启动游戏循环
void thunder_game_init() {
    // 初始化随机数种子，用于随机生成敌人位置
    srand(time(NULL));
    
    // 创建游戏屏幕（LVGL对象）
    game.screen = lv_obj_create(NULL);
    // 将创建的屏幕设置为当前活动屏幕
    lv_screen_load(game.screen);
    
    // 设置游戏屏幕背景颜色（深蓝色）
    lv_obj_set_style_bg_color(game.screen, lv_color_hex(0x000033), 0);
    
    // 创建游戏容器
    game.game_container = lv_obj_create(game.screen);
    lv_obj_set_size(game.game_container, THUNDER_GAME_AREA_WIDTH, THUNDER_GAME_AREA_HEIGHT);
    lv_obj_center(game.game_container);  // 居中显示
    lv_obj_set_style_bg_color(game.game_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(game.game_container, 2, 0);
    lv_obj_set_style_border_color(game.game_container, lv_color_hex(0xFFFFFF), 0);
    
    // 创建回到主菜单按钮
    lv_obj_t *esc_button = lv_button_create(game.screen);
    lv_obj_set_size(esc_button, 120, 50);  // 设置按钮大小
    lv_obj_set_align(esc_button, LV_ALIGN_TOP_RIGHT);  // 设置按钮位置（右上角）
    
    // 创建按钮标签
    lv_obj_t *label_ESC = lv_label_create(esc_button);
    lv_label_set_text(label_ESC, "ESC");   // 设置标签文本
    lv_obj_set_style_text_font(label_ESC, &lv_font_montserrat_20, 0);  // 设置字体
    lv_obj_center(label_ESC);              // 标签居中
    
    // 为按钮添加点击事件回调函数（返回主菜单）
    lv_obj_add_event_cb(esc_button, go_back_from_thunder, LV_EVENT_CLICKED, NULL);
    
    // 创建分数显示标签
    game.score_label = lv_label_create(game.screen);
    lv_obj_set_pos(game.score_label, 20, 80);  // 设置标签位置（左上角）
    lv_label_set_text_fmt(game.score_label, "Score: %d", game.player.score);  // 设置初始分数文本
    lv_obj_set_style_text_color(game.score_label, lv_color_hex(0xFFFFFF), 0);  // 设置文字颜色（白色）
    lv_obj_set_style_text_font(game.score_label, &lv_font_montserrat_20, 0);  // 设置字体大小
    
    // 创建生命值显示标签
    game.lives_label = lv_label_create(game.screen);
    lv_obj_set_pos(game.lives_label, 20, 110);  // 设置标签位置（左上角）
    lv_label_set_text_fmt(game.lives_label, "Lives: %d", game.player.lives);  // 设置初始生命值文本
    lv_obj_set_style_text_color(game.lives_label, lv_color_hex(0xFFFFFF), 0);  // 设置文字颜色（白色）
    lv_obj_set_style_text_font(game.lives_label, &lv_font_montserrat_20, 0);  // 设置字体大小
    
    // 计算游戏容器的位置
    int game_container_x = (THUNDER_WIDTH - THUNDER_GAME_AREA_WIDTH) / 2;
    int game_container_y = 0;
    
    // 创建自动射击按钮（游戏区域左侧）
    game.fire_button = lv_button_create(game.screen);
    lv_obj_set_size(game.fire_button, CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_HEIGHT);
    lv_obj_set_pos(game.fire_button, game_container_x - CONTROL_BUTTON_WIDTH - 20, THUNDER_GAME_AREA_HEIGHT / 2 - CONTROL_BUTTON_HEIGHT);  // 游戏区域左侧上方
    lv_obj_t *fire_label = lv_label_create(game.fire_button);
    lv_label_set_text(fire_label, "AUTO");  // 初始标签为自动射击
    lv_obj_set_style_text_font(fire_label, &lv_font_montserrat_24, 0);
    lv_obj_center(fire_label);
    lv_obj_add_event_cb(game.fire_button, fire_button_cb, LV_EVENT_CLICKED, NULL);
    
    // 初始化自动射击状态
    game.auto_fire_enabled = 0;
    
    // 创建左移按钮（游戏区域下方左侧）
    game.left_button = lv_button_create(game.screen);
    lv_obj_set_size(game.left_button, CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_HEIGHT);
    lv_obj_set_align(game.left_button, LV_ALIGN_BOTTOM_LEFT);  // 游戏区域下方左侧
    lv_obj_t *left_label = lv_label_create(game.left_button);
    lv_label_set_text(left_label, "LEFT");
    lv_obj_set_style_text_font(left_label, &lv_font_montserrat_24, 0);
    lv_obj_center(left_label);
    lv_obj_add_event_cb(game.left_button, left_button_cb, LV_EVENT_PRESSED, NULL);
    
    // 创建右移按钮（游戏区域下方右侧）
    game.right_button = lv_button_create(game.screen);
    lv_obj_set_size(game.right_button, CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_HEIGHT);
    lv_obj_set_align(game.right_button,LV_ALIGN_BOTTOM_RIGHT);  // 游戏区域下方右侧
    lv_obj_t *right_label = lv_label_create(game.right_button);
    lv_label_set_text(right_label, "RIGHT");
    lv_obj_set_style_text_font(right_label, &lv_font_montserrat_24, 0);
    lv_obj_center(right_label);
    lv_obj_add_event_cb(game.right_button, right_button_cb, LV_EVENT_PRESSED, NULL);
    
    // 初始化游戏核心元素
    init_player();    // 初始化玩家飞机
    init_bullets();   // 初始化子弹
    init_enemies();   // 初始化敌人
    
    // 设置游戏容器事件
    // 添加点击事件回调（用于发射子弹）
    lv_obj_add_event_cb(game.game_container, thunder_game_event_cb, LV_EVENT_CLICKED, NULL);
    // 添加手势事件回调（用于移动玩家飞机）
    lv_obj_add_event_cb(game.game_container, thunder_game_event_cb, LV_EVENT_GESTURE, NULL);
    // 设置游戏容器可点击
    lv_obj_add_flag(game.game_container, LV_OBJ_FLAG_CLICKABLE);
    
    // 创建自动射击定时器
    game.auto_fire_timer = lv_timer_create(auto_fire_timer_cb, AUTO_FIRE_INTERVAL, NULL);
    lv_timer_pause(game.auto_fire_timer);  // 初始暂停自动射击定时器
    
    // 启动游戏
    game.game_running = 1;  // 设置游戏运行状态
    // 创建游戏定时器，每50毫秒调用一次游戏更新函数
    game.game_timer = lv_timer_create(game_timer_cb, 50, NULL);
}