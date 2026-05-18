#include "ai.h"
#include "logic.h"
#include "ui.h"

//降序排列
static int compare_moves_desc(const void* a, const void* b) {
    return ((Move*)b)->score - ((Move*)a)->score;
}

//升序排列
static int compare_moves_asc(const void* a, const void* b) {
    return ((Move*)a)->score - ((Move*)b)->score;
}


static int alpha_beta(int depth, int alpha, int beta, int maximizing, int current_d) {
    ExMessage msg;
    // 修复：清空队列防止无响应
    while (peekmessage(&msg, EM_WINDOW | EM_MOUSE, true)) {}

    int score = evaluate_whole_board();
    // 修复：五子连珠的分数判定阈值
    if (score >= WIN_SCORE * 100) return score - current_d * 1000;
    if (score <= -WIN_SCORE * 100) return score + current_d * 1000;
    if (depth <= 0) return score;

    Move moves[361]; // 修复：防止数组越界
    int branch = MIN(get_candidates(moves), state.search_width);
    int best_val = maximizing ? -INF : INF;
    int player_to_place = maximizing ? 1 : 2;

    for (int i = 0; i < branch; i++) {
        board[moves[i].r][moves[i].c] = player_to_place;
        int v = alpha_beta(depth - 1, alpha, beta, 1 - maximizing, current_d + 1);
        board[moves[i].r][moves[i].c] = 0;

        if (maximizing) {
            best_val = MAX(best_val, v);
            alpha = MAX(alpha, best_val);
        }
        else {
            best_val = MIN(best_val, v);
            beta = MIN(beta, best_val);
        }
        if (beta <= alpha) break;
    }
    return best_val;
}

Move ai_move(int ai_player) {
    clock_t start = clock();
    Move best_m = { -1, -1, 0 };
    Move candidates[361];
    int cnt = get_candidates(candidates);

    if (cnt == 0) {
        best_m.r = BOARD_SIZE / 2;
        best_m.c = BOARD_SIZE / 2;
        return best_m;
    }

    int branch = MIN(cnt, state.search_width);

    for (int d = 1; d <= state.s_depth; d++) {
        int alpha = -INF, beta = INF;
        int best_s = (ai_player == 1) ? -INF : INF;
        Move temp_evals[361] = { 0 };

        for (int i = 0; i < branch; i++) {

            int r = candidates[i].r;
            int c = candidates[i].c;

            board[r][c] = ai_player;

            bool is_top_layer = (d == state.s_depth);

            if (is_top_layer) 
                draw_think_ghost(r, c, ai_player, false);

            int v = alpha_beta(d - 1, alpha, beta, (ai_player == 1 ? 0 : 1), 1);

            board[r][c] = 0;

            if (is_top_layer) 
                draw_think_ghost(r, c, ai_player, true);

            // 记录当前深度的打分
            temp_evals[i] = candidates[i];
            temp_evals[i].score = v;

            //剪枝更新
            if (ai_player == 1) {
                if (v > best_s) { best_s = v; best_m = candidates[i]; }
                alpha = MAX(alpha, best_s);
            }
            else {
                if (v < best_s) { best_s = v; best_m = candidates[i]; }
                beta = MIN(beta, best_s);
            }
        }

        if (ai_player == 1) {
            qsort(temp_evals, branch, sizeof(Move), compare_moves_desc);
        }
        else {
            qsort(temp_evals, branch, sizeof(Move), compare_moves_asc);
        }

        for (int i = 0; i < branch; i++) ai_eval_list[i] = temp_evals[i];
        
        ai_eval_count = branch;
        ai_eval_player = ai_player;

        // 提前绝杀判定
        if (abs(best_s) >= WIN_SCORE * 100) break; 
    }

    if (best_m.r != -1) {
        _stprintf_s(time_text, _T("耗时: %dms"), (int)(clock() - start));
        _stprintf_s(info_text, _T("AI 落子: (%c,%d)"), 'A' + best_m.r, best_m.c + 1);
    }

    return best_m;
}