#pragma once
#include "common.h"

// 声明logic.cpp开放函数

void init_score_table();

int evaluate_whole_board();

int get_point_score_local(int r, int c);

bool check_win(int r, int c, int p);

int get_candidates(Move* moves);