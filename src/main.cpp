#include "common.h"
#include "logic.h"
#include "ui.h"
#include "ai.h"

int board[BOARD_SIZE][BOARD_SIZE] = { 0 };
int tuple_score_table[243] = { 0 };
GameState state = { 4, 12, true, false, 0, 0, 0, 1, 0 };

Record move_history[361] = { 0 };
int history_count = 0;
Move ai_eval_list[361] = { 0 };
int ai_eval_count = 0;
int ai_eval_player = 0;

TCHAR info_text[128] = _T("等待落子...");
TCHAR time_text[64] = _T("AI 空闲");

const int DIRECTIONS[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };

// 执行悔棋操作
void undo_move() {
    if (history_count == 0) return;
    // 人机模式
    if (state.game_mode == 0) { 
        if (history_count >= 2) { // 必须退两步（玩家+AI）
            board[move_history[history_count - 1].r][move_history[history_count - 1].c] = 0;
            board[move_history[history_count - 2].r][move_history[history_count - 2].c] = 0;
            history_count -= 2;
        }
    }
	// AI/玩家 对战模式
    else { 
        history_count--;
        board[move_history[history_count].r][move_history[history_count].c] = 0;
        state.current_turn = move_history[history_count].player; 
    }
}

Move player_move() {
    ExMessage msg;
    while (1) {
        while (peekmessage(&msg, EM_MOUSE)) {
            
            if (msg.message != WM_LBUTTONDOWN) continue;

            //悔棋
            if (is_in_rect(msg.x, msg.y, WIN_W - 250, WIN_H - 70, 100, 40)) {
                Move undo_m = { -2, -1, 0 };
                return undo_m;
            }
			//退出
            if (is_in_rect(msg.x, msg.y, WIN_W - 130, WIN_H - 70, 100, 40)) {
                Move exit_m = { -3, -1, 0 };
                return exit_m;
            }

            // 求棋盘二维数组索引
            int j = (msg.x - MARGIN_X + CELL_SIZE / 2) / CELL_SIZE;
            int i = (msg.y - MARGIN_Y + CELL_SIZE / 2) / CELL_SIZE;

            // 检查是否为有效点击
            bool in_board_area = (msg.x >= MARGIN_X - CELL_SIZE / 2 && msg.x <= MARGIN_X + BOARD_PIXELS + CELL_SIZE / 2 &&
                msg.y >= MARGIN_Y - CELL_SIZE / 2 && msg.y <= MARGIN_Y + BOARD_PIXELS + CELL_SIZE / 2);

            if (in_board_area) {
                if (is_in_board(i, j) && board[i][j] == 0) {
                    Move m = { i, j, 0 };
                    return m;
                }
            }
            else {
                //拖动窗口
                SendMessage(GetHWnd(), WM_SYSCOMMAND, 0xF012, 0);
            }
        }
        Sleep(10); 
    }
}

void game_loop() {
   
    render_board_ui();
    int active = state.current_turn;

    //回合判断
    bool is_ai = false;
    if (state.game_mode == 0) is_ai = (state.is_ai_first && active == 1) || (!state.is_ai_first && active == 2);
    else if (state.game_mode == 1) is_ai = true;
    else if (state.game_mode == 2) is_ai = false;

    if (!is_ai) {
        _stprintf_s(info_text, _T("等待%s落子..."), active == 1 ? _T("黑方") : _T("白方"));
        _stprintf_s(time_text, _T("等待中"));
        // render_board_ui(); FlushBatchDraw();
        
    }

    Move last_m = is_ai ? ai_move(active) : player_move();

    //悔棋
    if (last_m.r == -2) {
        undo_move();
        return; 
    }
	//退出
    if (last_m.r == -3) {
        state.windows_index = 0;
        return; 
    }
    if (last_m.r == -1) {
        return; 
    }

    if (is_ai && state.game_mode == 1) Sleep(100);

    board[last_m.r][last_m.c] = active;

    // 记录历史
    move_history[history_count].r = last_m.r;
    move_history[history_count].c = last_m.c;
    move_history[history_count].player = active;
    history_count++;

    render_board_ui();

    if (check_win(last_m.r, last_m.c, active)) {
        process_game_over(is_ai, active);
    }
    else {
        state.current_turn = (active == 1) ? 2 : 1;
    }
}

int main() {
    initgraph(WIN_W, WIN_H);
    HWND hwnd = GetHWnd();
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    // 无边框
    SetWindowLong(hwnd, GWL_STYLE, style & ~WS_CAPTION & ~WS_THICKFRAME);

    int scr_w = GetSystemMetrics(SM_CXSCREEN);
    int scr_h = GetSystemMetrics(SM_CYSCREEN);
    int x_pos = (scr_w - WIN_W) / 2;
    int y_pos = (scr_h - WIN_H) / 2;

    SetWindowPos(hwnd, NULL, x_pos, y_pos, WIN_W, WIN_H, SWP_FRAMECHANGED);

    init_score_table();
    srand((unsigned)time(NULL));

    while (1) {
        if (state.windows_index == 0) menu_loop();
        else if (state.windows_index == 1) game_loop();
        Sleep(10);
    }
    return 0;
}