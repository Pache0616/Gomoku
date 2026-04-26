#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include <string.h>

//常量
#define BOARD_SIZE 19
#define CELL_SIZE 40
#define MARGIN 60
#define WIN_SIZE (MARGIN * 2 + (BOARD_SIZE - 1) * CELL_SIZE)

#define INF 200000000       
#define WIN_SCORE 100000    

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//数据结构
typedef struct {
    int r;
    int c;
    int score;
} Move;

//全局变量
int board[BOARD_SIZE][BOARD_SIZE] = { 0 };
int s_depth = 4;
int search_width = 12;
bool visual_enabled = true;
bool is_ai_first = false;
bool is_ai_vs_ai = false;
int player_wins = 0, ai_wins = 0;
int windows_index = 0;       // 0=菜单, 1=棋盘
int current_turn = 1;        // 1=黑棋, 2=白棋

volatile int current_score = 0;
TCHAR info_text[128] = _T("等待玩家落子...");
TCHAR time_text[64] = _T("AI 状态: 空闲");

// --- 函数声明 ---
bool check_win(int r, int c, int p);
void board_ui();
void msg_game_over(int is_ai_turn, bool active_player);

//得分评估
int evaluate_tuple(int b[5]) {
    int b_cnt = 0, w_cnt = 0;
    char s[6] = { 0 };

    // 统计并生成特征字符串
    for (int i = 0; i < 5; i++) {
        if (b[i] == 1) { b_cnt++; s[i] = 'B'; }
        else if (b[i] == 2) { w_cnt++; s[i] = 'W'; }
        else { s[i] = '0'; }
    }

    if ((b_cnt > 0 && w_cnt > 0) || (b_cnt == 0 && w_cnt == 0)) return 0;

    static const char* patterns[31] = {
        "B0000", "0B000", "00B00", "000B0", "0000B",
        "BB000", "0BB00", "00BB0", "000BB", "B0B00", "0B0B0", "00B0B", "B00B0", "0B00B", "B000B",
        "BBB00", "0BBB0", "00BBB", "BB0B0", "0BB0B", "B0BB0", "0B0BB", "BB00B", "B00BB", "B0B0B",
        "BBBB0", "BBB0B", "BB0BB", "B0BBB", "0BBBB",
        "BBBBB"
    };
    static const int scores[31] = {
        1, 1, 1, 1, 1,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        1000, 2000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
        WIN_SCORE, WIN_SCORE, WIN_SCORE, WIN_SCORE, WIN_SCORE, WIN_SCORE * 100
    };


    if (w_cnt > 0) {
        for (int i = 0; i < 5; i++) if (s[i] == 'W') s[i] = 'B';
    }

    //简化匹配逻辑
    for (int i = 0; i < 31; i++) {
        if (strcmp(s, patterns[i]) == 0) {
            return b_cnt > 0 ? scores[i] : -scores[i];
        }
    }
    return 0;
}

int evaluate_whole_board(int b[BOARD_SIZE][BOARD_SIZE]) {
    int total = 0, tuple[5];
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // 提取4个方向的五元组
            if (j <= BOARD_SIZE - 5) {
                for (int k = 0; k < 5; k++) tuple[k] = b[i][j + k];
                total += evaluate_tuple(tuple);
            }
            if (i <= BOARD_SIZE - 5) {
                for (int k = 0; k < 5; k++) tuple[k] = b[i + k][j];
                total += evaluate_tuple(tuple);
            }
            if (i <= BOARD_SIZE - 5 && j <= BOARD_SIZE - 5) {
                for (int k = 0; k < 5; k++) tuple[k] = b[i + k][j + k];
                total += evaluate_tuple(tuple);
            }
            if (i <= BOARD_SIZE - 5 && j >= 4) {
                for (int k = 0; k < 5; k++) tuple[k] = b[i + k][j - k];
                total += evaluate_tuple(tuple);
            }
        }
    }
    return total;
}

int get_point_score_local(int r, int c, int p) {
    int total_score = 0, tuple[5];
    int dirs[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

    for (int i = 0; i < 4; i++) {
        for (int start = -4; start <= 0; start++) {
            int tr = r + start * dirs[i][0];
            int tc = c + start * dirs[i][1];

            // 越界检查简化
            if (tr < 0 || tr + 4 * dirs[i][0] >= BOARD_SIZE) continue;
            if (tc < 0 || tc + 4 * dirs[i][1] >= BOARD_SIZE) continue;
            if (dirs[i][1] == -1 && (tc > BOARD_SIZE - 1 || tc - 4 < 0)) continue;

            for (int k = 0; k < 5; k++) {
                tuple[k] = board[tr + k * dirs[i][0]][tc + k * dirs[i][1]];
            }
            total_score += abs(evaluate_tuple(tuple));
        }
    }
    return total_score;
}

// 检查周围两格内是否有棋子
inline bool has_neighbor(int i, int j) {
    for (int r = MAX(0, i - 2); r <= MIN(BOARD_SIZE - 1, i + 2); r++) {
        for (int c = MAX(0, j - 2); c <= MIN(BOARD_SIZE - 1, j + 2); c++) {
            if (board[r][c] != 0) return true;
        }
    }
    return false;
}

// qsort
int compare_moves(const void* a, const void* b) {
    return ((Move*)b)->score - ((Move*)a)->score;
}

int get_candidates(Move* moves) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) continue;

            if (has_neighbor(i, j)) {
                moves[count].r = i;
                moves[count].c = j;
                moves[count].score = get_point_score_local(i, j, 1) + get_point_score_local(i, j, 2);
                count++;
            }
        }
    }

    qsort(moves, count, sizeof(Move), compare_moves);
    return count;
}

int alpha_beta(int depth, int alpha, int beta, int maximizing, int current_d) {
    int score = evaluate_whole_board(board);

    if (score >= WIN_SCORE) return score - current_d * 1000;
    if (score <= -WIN_SCORE) return score + current_d * 1000;
    if (depth <= 0) return score;

    Move moves[225];
    int count = get_candidates(moves);
    int branch = MIN(count, search_width);

    if (maximizing) {
        int max_v = -INF;
        for (int i = 0; i < branch; i++) {
            board[moves[i].r][moves[i].c] = 1;
            int v = alpha_beta(depth - 1, alpha, beta, 0, current_d + 1);
            board[moves[i].r][moves[i].c] = 0;

            max_v = MAX(v, max_v);
            alpha = MAX(v, alpha);
            if (beta <= alpha) break;
        }
        return max_v;
    }
    else {
        int min_v = INF;
        for (int i = 0; i < branch; i++) {
            board[moves[i].r][moves[i].c] = 2;
            int v = alpha_beta(depth - 1, alpha, beta, 1, current_d + 1);
            board[moves[i].r][moves[i].c] = 0;

            min_v = MIN(v, min_v);
            beta = MIN(v, beta);
            if (beta <= alpha) break;
        }
        return min_v;
    }
}

void draw_think_ghost(int r, int c, int p, bool is_clear) {
    if (!visual_enabled) return;
    int x = MARGIN + c * CELL_SIZE, y = MARGIN + r * CELL_SIZE;

    if (is_clear) {
        setfillcolor(RGB(243, 196, 107)); solidrectangle(x - 18, y - 18, x + 18, y + 18);
        setlinecolor(BLACK); line(x - 20, y, x + 20, y); line(x, y - 20, x, y + 20);
    }
    else {
        setlinecolor(p == 1 ? BLACK : WHITE); circle(x, y, 14); circle(x, y, 10);
    }
    FlushBatchDraw();
}

Move ai_move(int ai_player) {
    clock_t start = clock();
    Move final_move = { -1, -1, 0 };
    int final_score = (ai_player == 1) ? -INF : INF;

    Move candidates[225];
    int cnt = get_candidates(candidates);

    if (cnt == 0) {
        Move m = { BOARD_SIZE / 2, BOARD_SIZE / 2, 0 };
        board[m.r][m.c] = ai_player;
        return m;
    }
    int branch = MIN(cnt, search_width);
    printf("\n>>> %sAI 决策开始 (搜索深度: %d) <<<\n", (ai_player == 1) ? "黑棋" : "白棋", s_depth);

    for (int d = 1; d <= s_depth; d++) {
        int best_s = (ai_player == 1) ? -INF : INF;
        Move best_m = { -1, -1, 0 };
        int alpha = -INF, beta = INF;

        for (int i = 0; i < branch; i++) {
            board[candidates[i].r][candidates[i].c] = ai_player;
            if (d == s_depth) draw_think_ghost(candidates[i].r, candidates[i].c, ai_player, false);

            int v = alpha_beta(d - 1, alpha, beta, (ai_player == 1 ? 0 : 1), 1);
            board[candidates[i].r][candidates[i].c] = 0;

            if (d == s_depth) draw_think_ghost(candidates[i].r, candidates[i].c, ai_player, true);

            if (ai_player == 1) {
                if (v > best_s) { best_s = v; best_m = candidates[i]; }
                alpha = MAX(alpha, best_s);
            }
            else {
                if (v < best_s) { best_s = v; best_m = candidates[i]; }
                beta = MIN(beta, best_s);
            }
        }
        final_move = best_m;
        final_score = best_s;

        // 发现稳赢or必败局面提前结束
        if ((ai_player == 1 && final_score >= WIN_SCORE) || (ai_player == 2 && final_score <= -WIN_SCORE)) break;
    }

    if (final_move.r != -1) {
        board[final_move.r][final_move.c] = ai_player;
        current_score = final_score;
        _stprintf_s(time_text, _T("耗时: %dms"), (int)(clock() - start));
        _stprintf_s(info_text, _T("AI 落子: (%c,%d)"), 'A' + final_move.r, final_move.c + 1);
    }
    return final_move;
}

Move player_move() {
    ExMessage msg;
    while (1) {
        if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN) {
            int j = (msg.x - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
            int i = (msg.y - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
            if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE && board[i][j] == 0) {
                board[i][j] = current_turn;
                Move m = { i, j, 0 };
                return m;
            }
        }
    }
}

//界面
void menu_button(int x, int y, int w, int h, const TCHAR* text) {
    setfillcolor(RGB(225, 225, 225)); fillrectangle(x, y, x + w, y + h);
    settextcolor(BLACK); settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(x + (w - textwidth(text)) / 2, y + (h - textheight(text)) / 2, text);
}

void menu_draw() {
    TCHAR btn_txt[7][32];
    _stprintf_s(btn_txt[0], _T("开始游戏"));
    _stprintf_s(btn_txt[1], _T("搜索深度: %d"), s_depth);
    _stprintf_s(btn_txt[2], _T("搜索宽度: %d"), search_width);
    _stprintf_s(btn_txt[3], _T("可视化: %s"), visual_enabled ? _T("开") : _T("关"));
    _stprintf_s(btn_txt[4], _T("首手(黑棋): %s"), is_ai_first ? _T("AI") : _T("玩家"));
    _stprintf_s(btn_txt[5], _T("模式: %s"), is_ai_vs_ai ? _T("机机对战") : _T("人机对战"));
    _stprintf_s(btn_txt[6], _T("退出系统"));

    BeginBatchDraw();
    setbkcolor(RGB(250, 250, 250)); cleardevice();
    settextcolor(BLACK); settextstyle(35, 0, _T("黑体"));
    outtextxy(WIN_SIZE / 2 - 50, 60, _T("五子棋"));

    for (int i = 0; i < 7; i++) {
        menu_button(WIN_SIZE / 2 - 80, 160 + i * 60, 160, 40, btn_txt[i]);
    }
    FlushBatchDraw();
}

void menu_logic() {
    ExMessage msg;
    while (windows_index == 0) {
        menu_draw();
        if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN) {
            if (msg.x >= WIN_SIZE / 2 - 80 && msg.x <= WIN_SIZE / 2 + 80) {
                int btn_idx = (msg.y - 160) / 60;
                if (msg.y % 60 <= 40 && btn_idx >= 0 && btn_idx < 7) {
                    switch (btn_idx) {
                    case 0: {
                        memset(board, 0, sizeof(board));
                        windows_index = current_turn = 1;
                        break;
                    }
                    case 1: {
                        s_depth = (s_depth >= 6 ? 2 : s_depth + 2);
                        break;
                    }
                    case 2: {
                        search_width = (search_width >= 30 ? 8 : search_width + 4);
                        break;
                    }
                    case 3: {
                        visual_enabled = !visual_enabled;
                        break;
                    }
                    case 4: {
                        is_ai_first = !is_ai_first;
                        break;
                    }
                    case 5: {
                        is_ai_vs_ai = !is_ai_vs_ai;
                        break;
                    }
                    case 6: {
                        exit(0);
                    }
                    }
                }
            }
        }
    }
}

void board_ui() {
    BeginBatchDraw();
    setbkcolor(RGB(243, 196, 107)); cleardevice();
    setlinecolor(BLACK);
    for (int i = 0; i < BOARD_SIZE; i++) {
        line(MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (BOARD_SIZE - 1) * CELL_SIZE, MARGIN + i * CELL_SIZE);
        line(MARGIN + i * CELL_SIZE, MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (BOARD_SIZE - 1) * CELL_SIZE);
    }
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) {
                setfillcolor(board[i][j] == 1 ? BLACK : WHITE);
                solidcircle(MARGIN + j * CELL_SIZE, MARGIN + i * CELL_SIZE, 18);
            }
        }
    }
    TCHAR s[64];
    _stprintf_s(s, _T("玩家 %d : %d AI"), player_wins, ai_wins);
    settextcolor(BLACK); outtextxy(WIN_SIZE / 2 - textwidth(s) / 2, 10, s);
    outtextxy(20, WIN_SIZE - 30, info_text);
    outtextxy(WIN_SIZE - 150, WIN_SIZE - 30, time_text);
    FlushBatchDraw();
}

bool check_win(int r, int c, int p) {
    int d[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
    for (int i = 0; i < 4; i++) {
        int count = 1;
        // 正向延伸
        for (int step = 1; step <= 4; step++) {
            int nr = r + step * d[i][0], nc = c + step * d[i][1];
            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == p) count++;
            else break;
        }
        // 反向延伸
        for (int step = 1; step <= 4; step++) {
            int nr = r - step * d[i][0], nc = c - step * d[i][1];
            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == p) count++;
            else break;
        }
        if (count >= 5) return true;
    }
    return false;
}

void msg_game_over(int is_ai_turn, bool active_player) {
    if (!is_ai_vs_ai) {
        if (is_ai_turn) ai_wins++; else player_wins++;
        TCHAR msg[32];
        _stprintf_s(msg, _T("%s获胜！"), is_ai_turn ? _T("AI") : _T("玩家"));
        MessageBox(GetHWnd(), msg, _T("游戏结束"), 0);
    }
    else {
        MessageBox(GetHWnd(), active_player == 1 ? _T("黑方 AI 获胜！") : _T("白方 AI 获胜！"), _T("机机对战结束"), 0);
    }
    windows_index = 0;
}

void game_loop() {
    Move last_move = { -1, -1, 0 };
    while (windows_index == 1) {
        board_ui();
        int active_player = current_turn;
        bool is_ai_turn = is_ai_vs_ai || (is_ai_first && active_player == 1) || (!is_ai_first && active_player == 2);

        if (is_ai_turn) {
            last_move = ai_move(active_player);
            if (is_ai_vs_ai) Sleep(100);
        }
        else {
            last_move = player_move();
        }

        board_ui();

        if (last_move.r != -1 && check_win(last_move.r, last_move.c, active_player)) {
            msg_game_over(is_ai_turn, active_player);
        }
        else {
            current_turn = (active_player == 1) ? 2 : 1;
        }
    }
}

int main() {
    initgraph(WIN_SIZE, WIN_SIZE + 60);
    srand((unsigned)time(NULL));
    while (1) {
        if (windows_index == 0) menu_logic();
        else if (windows_index == 1) game_loop();
        Sleep(100);
    }
    return 0;
}
