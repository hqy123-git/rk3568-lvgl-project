#include "chinese_chess.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
extern const lv_font_t SiYuanHeiTi;
// 全局游戏实例
ChessGame chess_game;
lv_obj_t *chess_screen=NULL;

// 棋子图片路径数组
static const char* piece_image_paths[] = 
{
    "",                     // PIECE_NONE
    RED_KING_IMG_PATH,      // PIECE_RED_KING
    RED_GUARD_IMG_PATH,     // PIECE_RED_GUARD
    RED_ELEPHANT_IMG_PATH,  // PIECE_RED_ELEPHANT
    RED_HORSE_IMG_PATH,     // PIECE_RED_HORSE
    RED_CHARIOT_IMG_PATH,   // PIECE_RED_CHARIOT
    RED_CANNON_IMG_PATH,    // PIECE_RED_CANNON
    RED_PAWN_IMG_PATH,      // PIECE_RED_PAWN
    BLACK_KING_IMG_PATH,    // PIECE_BLACK_KING
    BLACK_GUARD_IMG_PATH,   // PIECE_BLACK_GUARD
    BLACK_ELEPHANT_IMG_PATH,// PIECE_BLACK_ELEPHANT
    BLACK_HORSE_IMG_PATH,   // PIECE_BLACK_HORSE
    BLACK_CHARIOT_IMG_PATH, // PIECE_BLACK_CHARIOT
    BLACK_CANNON_IMG_PATH,  // PIECE_BLACK_CANNON
    BLACK_PAWN_IMG_PATH     // PIECE_BLACK_PAWN
};

// 私有函数声明
static void create_piece(int index, PieceType type, int row, int col);
static void init_pieces(void);
static void board_to_screen_coords(int row, int col, int *screen_x, int *screen_y);
static bool screen_to_board_coords(int screen_x, int screen_y, int *row, int *col);
static void create_piece_ui(ChessPiece *piece);
static void piece_click_handler(lv_event_t *e);
static void board_click_handler(lv_event_t *e);
static void restart_handler(lv_event_t *e);
static void undo_handler(lv_event_t *e);
static void draw_debug_grid(void);

// 初始化游戏
void chess_game_init(void) 
{
    // 初始化棋盘状态
    for (int i = 0; i < BOARD_ROWS; i++) 
    {
        for (int j = 0; j < BOARD_COLS; j++) 
        {
            chess_game.board[i][j] = NULL;
        }
    }
    
    // 初始化游戏状态
    chess_game.state = GAME_STATE_INIT;
    chess_game.current_player = PIECE_COLOR_RED;
    chess_game.selected_piece = NULL;
    chess_game.is_piece_selected = false;
    chess_game.selected_row = -1;
    chess_game.selected_col = -1;
    
}

// 创建棋子
static void create_piece(int index, PieceType type, int row, int col) 
{
    if (index < 0 || index >= 32) return;
    
    ChessPiece *piece = &chess_game.pieces[index];
    piece->type = type;
    piece->color = (type >= PIECE_RED_KING && type <= PIECE_RED_PAWN) ? 
                   PIECE_COLOR_RED : PIECE_COLOR_BLACK;
    piece->row = row;
    piece->col = col;
    piece->alive = true;
    piece->img = NULL;
    
    // 添加到棋盘
    chess_game.board[row][col] = piece;
}

// 初始化棋子位置
static void init_pieces(void) 
{
    // 黑方棋子（上方）
    create_piece(0, PIECE_BLACK_CHARIOT, 0, 0);   // 车
    create_piece(1, PIECE_BLACK_HORSE, 0, 1);     // 马
    create_piece(2, PIECE_BLACK_ELEPHANT, 0, 2);  // 象
    create_piece(3, PIECE_BLACK_GUARD, 0, 3);     // 士
    create_piece(4, PIECE_BLACK_KING, 0, 4);      // 将
    create_piece(5, PIECE_BLACK_GUARD, 0, 5);     // 士
    create_piece(6, PIECE_BLACK_ELEPHANT, 0, 6);  // 象
    create_piece(7, PIECE_BLACK_HORSE, 0, 7);     // 马
    create_piece(8, PIECE_BLACK_CHARIOT, 0, 8);   // 车
    
    create_piece(9, PIECE_BLACK_CANNON, 2, 1);    // 炮
    create_piece(10, PIECE_BLACK_CANNON, 2, 7);   // 炮
    
    create_piece(11, PIECE_BLACK_PAWN, 3, 0);     // 卒
    create_piece(12, PIECE_BLACK_PAWN, 3, 2);     // 卒
    create_piece(13, PIECE_BLACK_PAWN, 3, 4);     // 卒
    create_piece(14, PIECE_BLACK_PAWN, 3, 6);     // 卒
    create_piece(15, PIECE_BLACK_PAWN, 3, 8);     // 卒
    
    // 红方棋子（下方）
    create_piece(16, PIECE_RED_PAWN, 6, 0);       // 兵
    create_piece(17, PIECE_RED_PAWN, 6, 2);       // 兵
    create_piece(18, PIECE_RED_PAWN, 6, 4);       // 兵
    create_piece(19, PIECE_RED_PAWN, 6, 6);       // 兵
    create_piece(20, PIECE_RED_PAWN, 6, 8);       // 兵
    
    create_piece(21, PIECE_RED_CANNON, 7, 1);     // 炮
    create_piece(22, PIECE_RED_CANNON, 7, 7);     // 炮
    
    create_piece(23, PIECE_RED_CHARIOT, 9, 0);    // 车
    create_piece(24, PIECE_RED_HORSE, 9, 1);      // 马
    create_piece(25, PIECE_RED_ELEPHANT, 9, 2);   // 相
    create_piece(26, PIECE_RED_GUARD, 9, 3);      // 仕
    create_piece(27, PIECE_RED_KING, 9, 4);       // 帅
    create_piece(28, PIECE_RED_GUARD, 9, 5);      // 仕
    create_piece(29, PIECE_RED_ELEPHANT, 9, 6);   // 相
    create_piece(30, PIECE_RED_HORSE, 9, 7);      // 马
    create_piece(31, PIECE_RED_CHARIOT, 9, 8);    // 车
    
}

// 棋盘坐标转屏幕坐标
static void board_to_screen_coords(int row, int col, int *screen_x, int *screen_y) 
{
    // 棋盘在屏幕上的绝对位置 - (200, 0)
    int board_x = BOARD_POS_X;  // 200
    int board_y = BOARD_POS_Y;  // 0
    
    // 棋盘内部有效区域大小（减去边框）
    int inner_width = BOARD_INNER_WIDTH;   // 480
    int inner_height = BOARD_INNER_HEIGHT; // 540
    
    // 每个交点的间距（8个间隔，9个交点）
    float col_spacing = (float)inner_width / (BOARD_COLS - 1);   // 480 / 8 = 60
    float row_spacing = (float)inner_height / (BOARD_ROWS - 1);  // 540 / 9 = 60
    
    // 计算交点在棋盘内部的位置（从边框内开始）
    float inner_x = col * col_spacing;
    float inner_y = row * row_spacing;
    
    // 加上边框偏移和棋盘位置
    *screen_x = board_x + BOARD_BORDER + (int)inner_x;
    *screen_y = board_y + BOARD_BORDER + (int)inner_y;
    
    // 调整棋子中心点（棋子中心对准交点）
    *screen_x = *screen_x - PIECE_SIZE / 2;
    *screen_y = *screen_y - PIECE_SIZE / 2;
    
    // 确保在屏幕范围内（1024x600）
    if (*screen_x < 0) *screen_x = 0;
    if (*screen_y < 0) *screen_y = 0;
    if (*screen_x > 1024 - PIECE_SIZE) *screen_x = 1024 - PIECE_SIZE;
    if (*screen_y > 600 - PIECE_SIZE) *screen_y = 600 - PIECE_SIZE;
}
// 屏幕坐标转棋盘坐标
static bool screen_to_board_coords(int screen_x, int screen_y, int *row, int *col) 
{
    // 棋盘在屏幕上的绝对位置 - 现在在(200, 0)
    int board_x = BOARD_POS_X;  // 200
    int board_y = BOARD_POS_Y;  // 0
    
    // 检查是否在棋盘图片范围内
    if (screen_x < board_x || screen_x >= board_x + BOARD_IMG_WIDTH ||
        screen_y < board_y || screen_y >= board_y + BOARD_IMG_HEIGHT) 
    {
        return false;
    }
    
    // 计算相对于棋盘内部区域的坐标（减去边框）
    float rel_x = (float)(screen_x - board_x - BOARD_BORDER);
    float rel_y = (float)(screen_y - board_y - BOARD_BORDER);
    
    // 检查是否在棋盘内部区域
    if (rel_x < 0 || rel_x >= BOARD_INNER_WIDTH ||
        rel_y < 0 || rel_y >= BOARD_INNER_HEIGHT) {
        return false;
    }
    
    // 每个交点的间距
    float col_spacing = (float)BOARD_INNER_WIDTH / (BOARD_COLS - 1);   // 480 / 8 = 60
    float row_spacing = (float)BOARD_INNER_HEIGHT / (BOARD_ROWS - 1);  // 540 / 9 = 60
    
    // 计算最近的交点（四舍五入）
    *col = (int)(rel_x / col_spacing + 0.5f);
    *row = (int)(rel_y / row_spacing + 0.5f);
    
    // 确保坐标在有效范围内
    if (*row < 0) *row = 0;
    if (*row >= BOARD_ROWS) *row = BOARD_ROWS - 1;
    if (*col < 0) *col = 0;
    if (*col >= BOARD_COLS) *col = BOARD_COLS - 1;
    
    return true;
}

// 创建棋子UI
static void create_piece_ui(ChessPiece *piece) 
{
    if (!piece || !piece->alive) return;
    
    // 计算屏幕位置
    int screen_x, screen_y;
    board_to_screen_coords(piece->row, piece->col, &screen_x, &screen_y);
    
    // 创建图像对象
    piece->img = lv_img_create(lv_scr_act());
    if (!piece->img) 
    {
        printf("Failed to create piece image!\n");
        return;
    }
    
    // 设置棋子图片
    const char* img_path = piece_image_paths[piece->type];
    lv_img_set_src(piece->img, img_path);
    
    // 设置棋子大小
    lv_obj_set_size(piece->img, PIECE_SIZE, PIECE_SIZE);
    
    // 设置绝对位置
    lv_obj_set_pos(piece->img, screen_x, screen_y);
    
    // 添加点击事件
    lv_obj_add_flag(piece->img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(piece->img, piece_click_handler, LV_EVENT_CLICKED, piece);
    
    // 设置样式
    lv_obj_set_style_border_width(piece->img, 0, 0);
    lv_obj_set_style_radius(piece->img, LV_RADIUS_CIRCLE, 0);
}

// 棋子点击事件处理
static void piece_click_handler(lv_event_t *e) 
{
    ChessPiece *piece = (ChessPiece*)lv_event_get_user_data(e);
    if (!piece || !piece->alive) return;
    
    printf("Piece clicked: type=%d at board(%d,%d)\n", piece->type, piece->row, piece->col);
    
    // 如果已经有选中的棋子，尝试移动
    if (chess_game.is_piece_selected && chess_game.selected_piece) 
    {
        printf("Trying to move from (%d,%d) to (%d,%d)\n",
               chess_game.selected_piece->row, chess_game.selected_piece->col,
               piece->row, piece->col);
        
        if (chess_game_move_piece(chess_game.selected_piece->row, 
                                 chess_game.selected_piece->col,
                                 piece->row, piece->col)) 
        {
            chess_game_deselect_piece();
        } 
        else 
        {
            chess_game_deselect_piece();
            if (piece->color == chess_game.current_player) 
            {
                chess_game_select_piece(piece->row, piece->col);
            }
        }
    } 
    else 
    {
        if (piece->color == chess_game.current_player) 
        {
            chess_game_select_piece(piece->row, piece->col);
        }
    }
}

// 棋盘点击事件处理
static void board_click_handler(lv_event_t *e) 
{
    lv_point_t point;
    lv_indev_get_point(lv_indev_get_act(), &point);
    
    printf("Board clicked at screen(%d, %d)\n", point.x, point.y);
    
    int row, col;
    if (screen_to_board_coords(point.x, point.y, &row, &col)) 
    {
        printf("Converted to board(%d, %d)\n", row, col);
        
        ChessPiece *piece = chess_game_get_piece_at(row, col);
        if (piece) {
            printf("Found piece, letting piece handler deal with it\n");
            return;
        }
        
        if (chess_game.is_piece_selected && chess_game.selected_piece) 
        {
            printf("Moving to empty square (%d, %d)\n", row, col);
            if (chess_game_move_piece(chess_game.selected_piece->row,
                                     chess_game.selected_piece->col,
                                     row, col)) 
            {
                chess_game_deselect_piece();
            }
        }
    } 
    else
    {
        printf("Click outside board or border area\n");
    }
}

// 重新开始按钮事件处理
static void restart_handler(lv_event_t *e) 
{
    chess_game_reset();
}

// 悔棋按钮事件处理
static void undo_handler(lv_event_t *e) 
{
    //......
}

// 创建游戏UI
void chess_game_create_ui(void)
{
    chess_screen=lv_obj_create(NULL);
    lv_screen_load(chess_screen);
    // 创建棋盘图片（绝对位置在(200, 0)）
    chess_game.board_img = lv_img_create(chess_screen);
    if (!chess_game.board_img) 
    {
        printf("Failed to create board image!\n");
        return;
    }
    
    lv_img_set_src(chess_game.board_img, BOARD_IMG_PATH);
    lv_obj_set_size(chess_game.board_img, BOARD_IMG_WIDTH, BOARD_IMG_HEIGHT);
    lv_obj_set_pos(chess_game.board_img, BOARD_POS_X, BOARD_POS_Y);  // (200, 0)
    
    // 添加棋盘点击事件
    lv_obj_add_flag(chess_game.board_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(chess_game.board_img, board_click_handler, 
                       LV_EVENT_CLICKED, NULL);
    
    // 初始化棋子位置
    init_pieces();
    
    // 创建所有棋子的UI
    for (int i = 0; i < 32; i++) 
    {
        if (chess_game.pieces[i].alive) 
        {
            create_piece_ui(&chess_game.pieces[i]);
        }
    }
    
    // 创建状态标签。坐标 (20,20)
    chess_game.status_label = lv_label_create(chess_screen);
    lv_obj_set_pos(chess_game.status_label, 20, 120);
    lv_obj_set_size(chess_game.status_label, 160, 60);
    lv_label_set_text(chess_game.status_label, "中国象棋");
    lv_obj_set_style_text_font(chess_game.status_label, &SiYuanHeiTi, 0);
    lv_obj_set_style_text_color(chess_game.status_label, lv_color_hex(0x8B4513), 0);

    
    // 创建回合标签。坐标 (20,100)
    chess_game.turn_label = lv_label_create(chess_screen);
    lv_obj_set_pos(chess_game.turn_label, 20, 200);
    lv_obj_set_size(chess_game.turn_label, 160, 60);
    lv_obj_set_style_text_font(chess_game.turn_label, &SiYuanHeiTi, 0);
    lv_obj_set_style_text_color(chess_game.turn_label, lv_color_hex(0x8B0000), 0);

    
    // 创建重新开始按钮
    chess_game.restart_btn = lv_btn_create(chess_screen);
    lv_obj_set_size(chess_game.restart_btn, 120, 40);
    // 位置：棋盘X坐标 + 棋盘宽度 + 20像素间距 = 780
    lv_obj_set_pos(chess_game.restart_btn, 780, 120);
    lv_obj_set_style_bg_color(chess_game.restart_btn, lv_color_hex(0x8B4513), 0);
    lv_obj_set_style_text_color(chess_game.restart_btn,lv_color_hex(0xffffff), 0);
    
    lv_obj_t *restart_label = lv_label_create(chess_game.restart_btn);
    lv_label_set_text(restart_label, "重新开始");
    lv_obj_set_style_text_font(restart_label, &SiYuanHeiTi, 0);
    lv_obj_center(restart_label);
    
    lv_obj_add_event_cb(chess_game.restart_btn, restart_handler, 
                       LV_EVENT_CLICKED, NULL);
    
    // 创建悔棋按钮
    chess_game.undo_btn = lv_btn_create(chess_screen);
    lv_obj_set_size(chess_game.undo_btn, 120, 40);
    // 位置：棋盘X坐标 + 棋盘宽度 + 20像素间距 = 780
    lv_obj_set_pos(chess_game.undo_btn,780, 200);
    lv_obj_set_style_bg_color(chess_game.undo_btn, lv_color_hex(0x8B4513), 0);
    lv_obj_set_style_text_color(chess_game.undo_btn, lv_color_hex(0xffffff), 0);
    
    lv_obj_t *undo_label = lv_label_create(chess_game.undo_btn);
    lv_label_set_text(undo_label, "悔棋");
    lv_obj_set_style_text_font(undo_label, &SiYuanHeiTi, 0);
    lv_obj_center(undo_label);
    
    lv_obj_add_event_cb(chess_game.undo_btn, undo_handler, 
                       LV_EVENT_CLICKED, NULL);
    
    // 更新游戏状态显示
    chess_game_update_status();
}

// 重置游戏
void chess_game_reset(void) 
{
    // 移除所有棋子UI
    for (int i = 0; i < 32; i++) 
    {
        if (chess_game.pieces[i].img) 
        {
            lv_obj_del(chess_game.pieces[i].img);
            chess_game.pieces[i].img = NULL;
        }
    }
    
    // 重新初始化游戏
    chess_game_init();
    init_pieces();
    
    // 重新创建棋子UI
    for (int i = 0; i < 32; i++) 
    {
        if (chess_game.pieces[i].alive) 
        {
            create_piece_ui(&chess_game.pieces[i]);
        }
    }
    // 更新状态
    chess_game_update_status();
}

// 选择棋子
void chess_game_select_piece(int row, int col) {
    ChessPiece *piece = chess_game_get_piece_at(row, col);
    if (!piece || piece->color != chess_game.current_player) 
    {
        return;
    }
    
    chess_game.selected_piece = piece;
    chess_game.selected_row = row;
    chess_game.selected_col = col;
    chess_game.is_piece_selected = true;
    
    chess_game_highlight_piece(piece);
    
    printf("Selected piece at (%d,%d)\n", row, col);
}

// 取消选择棋子
void chess_game_deselect_piece(void) 
{
    if (chess_game.selected_piece) {
        chess_game_unhighlight_piece(chess_game.selected_piece);
    }
    
    chess_game.selected_piece = NULL;
    chess_game.is_piece_selected = false;
    chess_game.selected_row = -1;
    chess_game.selected_col = -1;
    
    printf("Deselected piece\n");
}

// 高亮棋子
void chess_game_highlight_piece(ChessPiece *piece) 
{
    if (!piece || !piece->img) return;
    
    // 添加绿色边框
    lv_obj_set_style_border_color(piece->img, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(piece->img, 3, 0);
    lv_obj_set_style_border_opa(piece->img, LV_OPA_COVER, 0);
    
    // 添加阴影效果
    lv_obj_set_style_shadow_color(piece->img, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_shadow_width(piece->img, 10, 0);
    lv_obj_set_style_shadow_spread(piece->img, 3, 0);
    lv_obj_set_style_shadow_opa(piece->img, LV_OPA_30, 0);
}

// 取消高亮棋子
void chess_game_unhighlight_piece(ChessPiece *piece) 
{
    if (!piece || !piece->img) return;
    
    lv_obj_set_style_border_width(piece->img, 0, 0);
    lv_obj_set_style_shadow_width(piece->img, 0, 0);
}

// 移动棋子
bool chess_game_move_piece(int from_row, int from_col, int to_row, int to_col) 
{
    printf("Attempting move: from (%d,%d) to (%d,%d)\n", from_row, from_col, to_row, to_col);
    
    ChessPiece *piece = chess_game_get_piece_at(from_row, from_col);
    if (!piece || !piece->alive) {
        printf("No piece at source\n");
        return false;
    }
    
    if (piece->color != chess_game.current_player) {
        printf("Not your turn\n");
        return false;
    }
    
    if (!chess_game_is_valid_move(piece, to_row, to_col)) 
    {
        printf("Invalid move\n");
        return false;
    }
    
    ChessPiece *target = chess_game_get_piece_at(to_row, to_col);
    
    if (target && target->alive) 
    {
        if (target->color == piece->color) 
        {
            printf("Cannot capture your own piece\n");
            return false;
        }
        
        chess_game_remove_piece(target);
        
        if (target->type == PIECE_RED_KING || target->type == PIECE_BLACK_KING) 
        {
            chess_game.state = (piece->color == PIECE_COLOR_RED) ? 
                                GAME_STATE_RED_WIN : GAME_STATE_BLACK_WIN;
            printf("Checkmate! Game over.\n");
        }
    }
    
    chess_game.board[from_row][from_col] = NULL;
    chess_game.board[to_row][to_col] = piece;
    piece->row = to_row;
    piece->col = to_col;
    
    if (piece->img) 
    {
        int screen_x, screen_y;
        board_to_screen_coords(to_row, to_col, &screen_x, &screen_y);
        lv_obj_set_pos(piece->img, screen_x, screen_y);
        printf("Moved piece to screen(%d,%d)\n", screen_x, screen_y);
    }
    
    chess_game.current_player = (chess_game.current_player == PIECE_COLOR_RED) ?
                                  PIECE_COLOR_BLACK : PIECE_COLOR_RED;
    
    if (chess_game.state == GAME_STATE_RED_TURN || 
        chess_game.state == GAME_STATE_BLACK_TURN ||
        chess_game.state == GAME_STATE_INIT) {
        chess_game.state = (chess_game.current_player == PIECE_COLOR_RED) ?
                            GAME_STATE_RED_TURN : GAME_STATE_BLACK_TURN;
    }
    
    chess_game_update_status();
    return true;
}

// 移除棋子
void chess_game_remove_piece(ChessPiece *piece) 
{
    if (!piece) return;
    
    piece->alive = false;
    chess_game.board[piece->row][piece->col] = NULL;
    
    if (piece->img) 
    {
        lv_obj_add_flag(piece->img, LV_OBJ_FLAG_HIDDEN);
    }
}

// 获取指定位置的棋子
ChessPiece* chess_game_get_piece_at(int row, int col) 
{
    if (!chess_game_is_within_board(row, col)) {
        return NULL;
    }
    
    ChessPiece *piece = chess_game.board[row][col];
    if (piece && piece->alive) {
        return piece;
    }
    
    return NULL;
}

// 检查是否在棋盘内
bool chess_game_is_within_board(int row, int col) 
{
    return (row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS);
}

// 检查是否为同一颜色
bool chess_game_is_same_color(ChessPiece *p1, ChessPiece *p2) 
{
    if (!p1 || !p2) return false;
    return p1->color == p2->color;
}

// 统计两点之间的棋子数量
int chess_game_count_pieces_between(int from_row, int from_col, int to_row, int to_col) 
{
    int count = 0;
    
    if (from_row == to_row) 
    {
        int step = (to_col > from_col) ? 1 : -1;
        for (int col = from_col + step; col != to_col; col += step) 
        {
            if (chess_game_get_piece_at(from_row, col)) 
            {
                count++;
            }
        }
    } else if (from_col == to_col) 
    {
        int step = (to_row > from_row) ? 1 : -1;
        for (int row = from_row + step; row != to_row; row += step) 
        {
            if (chess_game_get_piece_at(row, from_col)) 
            {
                count++;
            }
        }
    }
    
    return count;
}

// 检查移动是否合法
bool chess_game_is_valid_move(ChessPiece *piece, int to_row, int to_col) 
{
    if (!piece || !piece->alive) return false;
    if (!chess_game_is_within_board(to_row, to_col)) return false;
    if (to_row == piece->row && to_col == piece->col) return false;
    
    ChessPiece *target = chess_game_get_piece_at(to_row, to_col);
    if (target && chess_game_is_same_color(piece, target)) 
    {
        return false;
    }
    
    switch (piece->type) 
    {
        case PIECE_RED_KING:
        case PIECE_BLACK_KING:
            return chess_game_validate_king_move(piece, to_row, to_col);
            
        case PIECE_RED_GUARD:
        case PIECE_BLACK_GUARD:
            return chess_game_validate_guard_move(piece, to_row, to_col);
            
        case PIECE_RED_ELEPHANT:
        case PIECE_BLACK_ELEPHANT:
            return chess_game_validate_elephant_move(piece, to_row, to_col);
            
        case PIECE_RED_HORSE:
        case PIECE_BLACK_HORSE:
            return chess_game_validate_horse_move(piece, to_row, to_col);
            
        case PIECE_RED_CHARIOT:
        case PIECE_BLACK_CHARIOT:
            return chess_game_validate_chariot_move(piece, to_row, to_col);
            
        case PIECE_RED_CANNON:
        case PIECE_BLACK_CANNON:
            return chess_game_validate_cannon_move(piece, to_row, to_col);
            
        case PIECE_RED_PAWN:
        case PIECE_BLACK_PAWN:
            return chess_game_validate_pawn_move(piece, to_row, to_col);
            
        default:
            return false;
    }
}

// 将/帅移动验证
bool chess_game_validate_king_move(ChessPiece *piece, int to_row, int to_col) 
{
    int palace_top = (piece->color == PIECE_COLOR_RED) ? 7 : 0;
    int palace_bottom = (piece->color == PIECE_COLOR_RED) ? 9 : 2;
    int palace_left = 3;
    int palace_right = 5;
    
    if (to_row < palace_top || to_row > palace_bottom || 
        to_col < palace_left || to_col > palace_right) 
        {
        return false;
    }
    
    int row_diff = abs(to_row - piece->row);
    int col_diff = abs(to_col - piece->col);
    
    return (row_diff == 1 && col_diff == 0) || (row_diff == 0 && col_diff == 1);
}

// 士/仕移动验证
bool chess_game_validate_guard_move(ChessPiece *piece, int to_row, int to_col) 
{
    int palace_top = (piece->color == PIECE_COLOR_RED) ? 7 : 0;
    int palace_bottom = (piece->color == PIECE_COLOR_RED) ? 9 : 2;
    int palace_left = 3;
    int palace_right = 5;
    
    if (to_row < palace_top || to_row > palace_bottom || 
        to_col < palace_left || to_col > palace_right) 
    {
        return false;
    }
    
    int row_diff = abs(to_row - piece->row);
    int col_diff = abs(to_col - piece->col);
    
    return (row_diff == 1 && col_diff == 1);
}

// 相/象移动验证
bool chess_game_validate_elephant_move(ChessPiece *piece, int to_row, int to_col) 
{
    if ((piece->color == PIECE_COLOR_RED && to_row < 5) ||
        (piece->color == PIECE_COLOR_BLACK && to_row > 4)) 
    {
        return false;
    }
    
    int row_diff = abs(to_row - piece->row);
    int col_diff = abs(to_col - piece->col);
    
    if (row_diff != 2 || col_diff != 2) 
    {
        return false;
    }
    
    int center_row = (piece->row + to_row) / 2;
    int center_col = (piece->col + to_col) / 2;
    
    return !chess_game_get_piece_at(center_row, center_col);
}

// 马移动验证
bool chess_game_validate_horse_move(ChessPiece *piece, int to_row, int to_col) 
{
    int row_diff = abs(to_row - piece->row);
    int col_diff = abs(to_col - piece->col);
    
    if (!((row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2))) 
    {
        return false;
    }
    
    if (row_diff == 2) 
    {
        int middle_row = (piece->row + to_row) / 2;
        if (chess_game_get_piece_at(middle_row, piece->col)) 
        {
            return false;
        }
    } else {
        int middle_col = (piece->col + to_col) / 2;
        if (chess_game_get_piece_at(piece->row, middle_col)) 
        {
            return false;
        }
    }
    
    return true;
}

// 车移动验证
bool chess_game_validate_chariot_move(ChessPiece *piece, int to_row, int to_col) 
{
    if (piece->row != to_row && piece->col != to_col) 
    {
        return false;
    }
    
    return chess_game_count_pieces_between(piece->row, piece->col, to_row, to_col) == 0;
}

// 炮移动验证
bool chess_game_validate_cannon_move(ChessPiece *piece, int to_row, int to_col) 
{
    if (piece->row != to_row && piece->col != to_col) 
    {
        return false;
    }
    
    ChessPiece *target = chess_game_get_piece_at(to_row, to_col);
    int piece_count = chess_game_count_pieces_between(piece->row, piece->col, to_row, to_col);
    
    if (!target) 
    {
        return piece_count == 0;
    } 
    else 
    {
        return piece_count == 1;
    }
}

// 兵/卒移动验证
bool chess_game_validate_pawn_move(ChessPiece *piece, int to_row, int to_col) 
{
    int row_diff = to_row - piece->row;
    int col_diff = abs(to_col - piece->col);
    
    if (piece->color == PIECE_COLOR_RED) 
    {
        if (row_diff > 0) return false;
        
        if (piece->row > 4) 
        {
            return (row_diff == -1 && col_diff == 0);
        } 
        else 
        {
            return (row_diff == -1 && col_diff == 0) || 
                   (row_diff == 0 && col_diff == 1);
        }
    } 
    else 
    {
        if (row_diff < 0) return false;
        
        if (piece->row < 5) 
        {
            return (row_diff == 1 && col_diff == 0);
        } 
        else 
        {
            return (row_diff == 1 && col_diff == 0) || 
                   (row_diff == 0 && col_diff == 1);
        }
    }
}

// 更新游戏状态显示
void chess_game_update_status(void) 
{
    if (!chess_game.status_label || !chess_game.turn_label) return;
    const char *status_text = "";
    const char *turn_text = "";
    
    switch (chess_game.state) 
    {
        case GAME_STATE_INIT:
            status_text = "中国象棋";
            turn_text = "红方先行";
            break;
            
        case GAME_STATE_RED_TURN:
            status_text = "中国象棋";
            turn_text = "红方执子";
            break;
            
        case GAME_STATE_BLACK_TURN:
            status_text = "中国象棋";
            turn_text = "黑方执子";
            break;
            
        case GAME_STATE_RED_WIN:
            status_text = "红方胜利!";
            turn_text = "游戏结束";
            break;
            
        case GAME_STATE_BLACK_WIN:
            status_text = "黑方胜利!";
            turn_text = "游戏结束";
            break;
            
        case GAME_STATE_DRAW:
            status_text = "和棋!";
            turn_text = "游戏结束";
            break;
    }
    //printf("%s,%s\n",status_text,turn_text);
    lv_label_set_text(chess_game.status_label, status_text);
    lv_label_set_text(chess_game.turn_label, turn_text);
}

// 销毁游戏
void chess_game_destroy(void) 
{
    for (int i = 0; i < 32; i++) 
    {
        if (chess_game.pieces[i].img) 
        {
            lv_obj_del(chess_game.pieces[i].img);
            chess_game.pieces[i].img = NULL;
        }
    }
    
    if (chess_game.board_img) 
    {
        lv_obj_del(chess_game.board_img);
        chess_game.board_img = NULL;
    }
    
    if (chess_game.status_label) 
    {
        lv_obj_del(chess_game.status_label);
        chess_game.status_label = NULL;
    }
    
    if (chess_game.turn_label) 
    {
        lv_obj_del(chess_game.turn_label);
        chess_game.turn_label = NULL;
    }
    
    if (chess_game.restart_btn) 
    {
        lv_obj_del(chess_game.restart_btn);
        chess_game.restart_btn = NULL;
    }
    
    if (chess_game.undo_btn) 
    {
        lv_obj_del(chess_game.undo_btn);
        chess_game.undo_btn = NULL;
    }
    
    memset(&chess_game, 0, sizeof(ChessGame));

}