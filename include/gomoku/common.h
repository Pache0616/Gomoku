#pragma once
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include <string.h>

//常量定义
#define BOARD_SIZE 19
#define CELL_SIZE 40
#define BOARD_PIXELS ((BOARD_SIZE - 1) * CELL_SIZE) 

// 界面布局尺寸
#define MARGIN_X 260  
#define MARGIN_Y 60 
#define WIN_W (MARGIN_X * 2 + BOARD_PIXELS)     
#define WIN_H (MARGIN_Y * 2 + BOARD_PIXELS + 30)

//评估函数
#define INF 200000000       
#define WIN_SCORE 100000    

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//落子得分
typedef struct {
    int r;
    int c;
    int score;
} Move;

//历史记录节点
typedef struct {
    int r, c, player;
} Record;

//游戏状态
typedef struct {
    int s_depth;
    int search_width;
    bool visual_enabled;
    bool is_ai_first;
    int game_mode;
    int player_wins;
    int ai_wins;
    int current_turn;
    int windows_index;
} GameState;


extern int board[BOARD_SIZE][BOARD_SIZE];
extern int tuple_score_table[243];
extern GameState state;

// 走子记录
extern Record move_history[361]; 
extern int history_count;

// AI分数记录
extern Move ai_eval_list[361];   
extern int ai_eval_count;
extern int ai_eval_player;

//文本栈
extern TCHAR info_text[128];
extern TCHAR time_text[64];
extern const int DIRECTIONS[4][2];

//内联函数

//棋盘点击检测1
inline bool is_in_board(int r, int c) {
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}

//棋盘点击检测2
inline bool is_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

//五元组索引
inline int get_tuple_index(int b[5]) {
    return b[0] * 81 + b[1] * 27 + b[2] * 9 + b[3] * 3 + b[4];
}


