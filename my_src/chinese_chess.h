#ifndef CHINESE_CHESS_H
#define CHINESE_CHESS_H

#include "lvgl.h"
#include <stdbool.h>

// 棋盘配置
#define BOARD_ROWS 10        // 10行交点
#define BOARD_COLS 9         // 9列交点
#define PIECE_SIZE 54        // 棋子图片大小

// 棋盘图片尺寸和边框
#define BOARD_IMG_WIDTH 540
#define BOARD_IMG_HEIGHT 600
#define BOARD_BORDER 30      // 边框宽度


// 棋盘在屏幕上的绝对位置
#define BOARD_POS_X 200    
#define BOARD_POS_Y 0      

// 计算棋盘内部有效区域
#define BOARD_INNER_WIDTH (BOARD_IMG_WIDTH - 2 * BOARD_BORDER)    // 480
#define BOARD_INNER_HEIGHT (BOARD_IMG_HEIGHT - 2 * BOARD_BORDER)  // 540

// 游戏状态
typedef enum {
    GAME_STATE_INIT,
    GAME_STATE_RED_TURN,
    GAME_STATE_BLACK_TURN,
    GAME_STATE_RED_WIN,
    GAME_STATE_BLACK_WIN,
    GAME_STATE_DRAW
} GamechessState;

// 棋子类型
typedef enum {
    PIECE_NONE = 0,
    PIECE_RED_KING,      // 帅
    PIECE_RED_GUARD,     // 仕
    PIECE_RED_ELEPHANT,  // 相
    PIECE_RED_HORSE,     // 马
    PIECE_RED_CHARIOT,   // 车
    PIECE_RED_CANNON,    // 炮
    PIECE_RED_PAWN,      // 兵
    PIECE_BLACK_KING,    // 将
    PIECE_BLACK_GUARD,   // 士
    PIECE_BLACK_ELEPHANT,// 象
    PIECE_BLACK_HORSE,   // 马
    PIECE_BLACK_CHARIOT, // 车
    PIECE_BLACK_CANNON,  // 炮
    PIECE_BLACK_PAWN     // 卒
} PieceType;

// 棋子颜色
typedef enum {
    PIECE_COLOR_NONE,
    PIECE_COLOR_RED,
    PIECE_COLOR_BLACK
} PieceColor;

// 棋子结构
typedef struct {
    PieceType type;
    PieceColor color;
    int row;          // 行位置 (0-9)
    int col;          // 列位置 (0-8)
    lv_obj_t *img;    // LVGL图像对象
    bool alive;       // 是否存活
} ChessPiece;

// 游戏主结构
typedef struct {
    ChessPiece pieces[32];           // 所有棋子
    ChessPiece *board[BOARD_ROWS][BOARD_COLS]; // 棋盘状态
    GamechessState state;
    PieceColor current_player;
    ChessPiece *selected_piece;      // 当前选中的棋子
    bool is_piece_selected;
    int selected_row;
    int selected_col;
    
    // UI元素
    lv_obj_t *board_img;      // 棋盘图片
    lv_obj_t *status_label;   // 状态标签
    lv_obj_t *turn_label;     // 回合标签
    lv_obj_t *restart_btn;    // 重新开始按钮
    lv_obj_t *undo_btn;       // 悔棋按钮
    
} ChessGame;

// 全局游戏实例
extern ChessGame chess_game;

// 图片路径
#define BOARD_IMG_PATH "A:/hqy/chess_board_simple.png"
#define RED_KING_IMG_PATH "A:/hqy/red_king.png"
#define RED_GUARD_IMG_PATH "A:/hqy/red_guard.png"
#define RED_ELEPHANT_IMG_PATH "A:/hqy/red_elephant.png"
#define RED_HORSE_IMG_PATH "A:/hqy/red_horse.png"
#define RED_CHARIOT_IMG_PATH "A:/hqy/red_chariot.png"
#define RED_CANNON_IMG_PATH "A:/hqy/red_cannon.png"
#define RED_PAWN_IMG_PATH "A:/hqy/red_pawn.png"
#define BLACK_KING_IMG_PATH "A:/hqy/black_king.png"
#define BLACK_GUARD_IMG_PATH "A:/hqy/black_guard.png"
#define BLACK_ELEPHANT_IMG_PATH "A:/hqy/black_elephant.png"
#define BLACK_HORSE_IMG_PATH "A:/hqy/black_horse.png"
#define BLACK_CHARIOT_IMG_PATH "A:/hqy/black_chariot.png"
#define BLACK_CANNON_IMG_PATH "A:/hqy/black_cannon.png"
#define BLACK_PAWN_IMG_PATH "A:/hqy/black_pawn.png"

// 函数声明
void chess_game_init(void);
void chess_game_create_ui(void);
void chess_game_reset(void);
bool chess_game_move_piece(int from_row, int from_col, int to_row, int to_col);
void chess_game_select_piece(int row, int col);
void chess_game_deselect_piece(void);
void chess_game_update_status(void);
void chess_game_destroy(void);

// 移动验证函数
bool chess_game_is_valid_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_king_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_guard_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_elephant_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_horse_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_chariot_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_cannon_move(ChessPiece *piece, int to_row, int to_col);
bool chess_game_validate_pawn_move(ChessPiece *piece, int to_row, int to_col);

// 辅助函数
bool chess_game_is_within_board(int row, int col);
bool chess_game_is_same_color(ChessPiece *p1, ChessPiece *p2);
int chess_game_count_pieces_between(int from_row, int from_col, int to_row, int to_col);
ChessPiece* chess_game_get_piece_at(int row, int col);
void chess_game_highlight_piece(ChessPiece *piece);
void chess_game_unhighlight_piece(ChessPiece *piece);
void chess_game_remove_piece(ChessPiece *piece);

#endif // CHINESE_CHESS_H