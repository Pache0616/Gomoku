#pragma once
#include "common.h"
#include <graphics.h> 
#include <easyx.h>

// 声明ui.cpp开放函数

void draw_think_ghost(int r, int c, int p, bool is_clear);

void render_board_ui();

void menu_loop();

void process_game_over(int is_ai_turn, int active_player);