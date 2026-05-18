#include "ui.h"

// 绘制棋盘线
static void draw_grid() {
    setlinecolor(BLACK);
    for (int i = 0; i < BOARD_SIZE; i++) {
        line(MARGIN_X, MARGIN_Y + i * CELL_SIZE, MARGIN_X + BOARD_PIXELS, MARGIN_Y + i * CELL_SIZE);
        line(MARGIN_X + i * CELL_SIZE, MARGIN_Y, MARGIN_X + i * CELL_SIZE, MARGIN_Y + BOARD_PIXELS);
    }
}

// 绘制棋子
static void draw_pieces() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != 0) {
                setfillcolor(board[i][j] == 1 ? BLACK : WHITE);
                solidcircle(MARGIN_X + j * CELL_SIZE, MARGIN_Y + i * CELL_SIZE, 18);
            }
        }
    }
}

//绘制数据面板
static void draw_info_panels() {
    settextcolor(BLACK);
    settextstyle(20, 0, _T("微软雅黑"));

    // 历史记录
    outtextxy(30, MARGIN_Y, _T("走子轮次记录:"));
    int start_idx = MAX(0, history_count - 28);
    for (int i = start_idx; i < history_count; i++) {
        TCHAR buf[64];
        _stprintf_s(buf, _T("[%03d] %s : (%c, %d)"), i + 1, move_history[i].player == 1 ? _T("黑") : _T("白"), 'A' + move_history[i].r, move_history[i].c + 1);
        outtextxy(30, MARGIN_Y + 40 + (i - start_idx) * 22, buf);
    }

    //AI 评估
    if (ai_eval_count > 0) {
        TCHAR buf2[64];
        _stprintf_s(buf2, _T("AI(%s) 顶层打分分布:"), ai_eval_player == 1 ? _T("黑") : _T("白"));
        outtextxy(MARGIN_X + BOARD_PIXELS + 30, MARGIN_Y, buf2);

        for (int i = 0; i < MIN(28, ai_eval_count); i++) {
            TCHAR buf[64];
            _stprintf_s(buf, _T("(%c, %d)   分数: %d"), 'A' + ai_eval_list[i].r, ai_eval_list[i].c + 1, ai_eval_list[i].score);
            outtextxy(MARGIN_X + BOARD_PIXELS + 30, MARGIN_Y + 40 + i * 22, buf);
        }
    }
}

//绘制按钮
static void draw_button(int x, int y, int w, int h, const TCHAR* text) {
    
    //透明度
    DWORD* pMem = GetImageBuffer();
    int win_w = getwidth();

    COLORREF btn_color = RGB(225, 225, 225); 
    //0.0为完全透明，1.0为完全不透明。
    float alpha = 0.6f;

    int r1 = GetRValue(btn_color);
    int g1 = GetGValue(btn_color);
    int b1 = GetBValue(btn_color);

    
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {

            DWORD c = pMem[i * win_w + j];

            int r2 = (c >> 16) & 0xFF;
            int g2 = (c >> 8) & 0xFF;
            int b2 = c & 0xFF;

            int r = (int)(r1 * alpha + r2 * (1.0f - alpha));
            int g = (int)(g1 * alpha + g2 * (1.0f - alpha));
            int b = (int)(b1 * alpha + b2 * (1.0f - alpha));

            pMem[i * win_w + j] = (r << 16) | (g << 8) | b;
        }
    }

    // 按钮文字
    settextcolor(BLACK);
    settextstyle(20, 0, _T("微软雅黑"));
    setbkmode(TRANSPARENT); 
    outtextxy(x + (w - textwidth(text)) / 2, y + (h - textheight(text)) / 2, text);
}

void draw_think_ghost(int r, int c, int p, bool is_clear) {
    if (!state.visual_enabled) return;
    int x = MARGIN_X + c * CELL_SIZE, y = MARGIN_Y + r * CELL_SIZE; 
    if (is_clear) {
        setfillcolor(RGB(243, 196, 107)); solidrectangle(x - 18, y - 18, x + 18, y + 18);
        setlinecolor(BLACK); line(x - 20, y, x + 20, y); line(x, y - 20, x, y + 20);
    }
    else {
        setlinecolor(p == 1 ? BLACK : WHITE); circle(x, y, 14); circle(x, y, 10);
    }
    FlushBatchDraw();
}

//菜单点击结果
static void handle_menu_click(int btn_index) {
    switch (btn_index) {
    case 0:
        memset(board, 0, sizeof(board));
        history_count = 0;
        ai_eval_count = 0;
        state.windows_index = state.current_turn = 1;
        break;
    case 1: state.s_depth = (state.s_depth >= 6 ? 2 : state.s_depth + 2); break;
    case 2: state.search_width = (state.search_width >= 30 ? 8 : state.search_width + 4); break;
    case 3: state.visual_enabled = !state.visual_enabled; break;
    case 4: state.is_ai_first = !state.is_ai_first; break;
    case 5: state.game_mode = (state.game_mode + 1) % 3; break;
    case 6: exit(0);
    }
}

void render_board_ui() {
    BeginBatchDraw();
    setbkcolor(RGB(243, 196, 107));
    cleardevice();

    //棋盘
    draw_grid();
    draw_pieces();
    draw_info_panels();

    // 悔棋与退出按钮
    draw_button(WIN_W - 250, WIN_H - 70, 100, 40, _T("悔棋"));
    draw_button(WIN_W - 130, WIN_H - 70, 100, 40, _T("退出"));

    // 状态文字
    TCHAR s[64];
    if (state.game_mode == 0) _stprintf_s(s, _T("玩家 %d : %d AI"), state.player_wins, state.ai_wins);
    else if (state.game_mode == 1) _stprintf_s(s, _T("黑方AI %d : %d 白方AI"), state.player_wins, state.ai_wins);
    else _stprintf_s(s, _T("黑方玩家 %d : %d 白方玩家"), state.player_wins, state.ai_wins);

    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(WIN_W / 2 - textwidth(s) / 2, 20, s);
    outtextxy(MARGIN_X, WIN_H - 40, info_text);
    outtextxy(MARGIN_X + BOARD_PIXELS - 150, WIN_H - 40, time_text);

    FlushBatchDraw();
}

void process_game_over(int is_ai_turn, int active_player) {
    if (state.game_mode == 0) {
        if (is_ai_turn) state.ai_wins++; else state.player_wins++;
        TCHAR msg[32]; _stprintf_s(msg, _T("%s获胜！"), is_ai_turn ? _T("AI") : _T("玩家"));
        MessageBox(GetHWnd(), msg, _T("对局结束"), 0);
    }
    else {
        if (active_player == 1) state.player_wins++; else state.ai_wins++;
        MessageBox(GetHWnd(), active_player == 1 ? _T("黑方 获胜！") : _T("白方 获胜！"), _T("对战结束"), 0);
    }
    state.windows_index = 0;
}

void menu_loop() {
    TCHAR btn_txt[7][32];
    static IMAGE img_bk;
    static bool is_loaded = false;
    if (!is_loaded) {
        loadimage(&img_bk, _T("menu_background.png"), WIN_W, WIN_H);
        is_loaded = true;
    }

    const TCHAR* mode_str[] = { _T("人机对战"), _T("ai自我对战(beta)"), _T("玩家对战") };

    // 按钮文本配置
    _stprintf_s(btn_txt[0], _T("开始游戏"));
    _stprintf_s(btn_txt[1], _T("搜索深度: %d"), state.s_depth);
    _stprintf_s(btn_txt[2], _T("搜索宽度: %d"), state.search_width);
    _stprintf_s(btn_txt[3], _T("可视化: %s"), state.visual_enabled ? _T("开") : _T("关"));
    _stprintf_s(btn_txt[4], _T("首手(黑棋): %s"), state.is_ai_first ? _T("AI") : _T("玩家"));
    _stprintf_s(btn_txt[5], _T("模式: %s"), mode_str[state.game_mode]);
    _stprintf_s(btn_txt[6], _T("退出系统"));

    BeginBatchDraw();
    putimage(0, 0, &img_bk);
    setbkmode(TRANSPARENT);
    settextcolor(BLACK); settextstyle(35, 0, _T("黑体"));
    outtextxy(WIN_W / 2 - 50, 60, _T("五子棋"));

    int btn_x = WIN_W / 2 - 80, btn_w = 160, btn_h = 40;
    for (int i = 0; i < 7; i++) {
        draw_button(btn_x, 160 + i * 60, btn_w, btn_h, btn_txt[i]);
    }
    FlushBatchDraw();

    ExMessage msg;
    while (peekmessage(&msg, EM_MOUSE)) {

        if (msg.message != WM_LBUTTONDOWN) continue;

        bool clicked_btn = false;
        for (int i = 0; i < 7; i++) {
            if (is_in_rect(msg.x, msg.y, btn_x, 160 + i * 60, btn_w, btn_h)) {
                handle_menu_click(i);
                clicked_btn = true;
                break;
            }
        }

        // 拖动窗口
        if (!clicked_btn) {
            SendMessage(GetHWnd(), WM_SYSCOMMAND, 0xF012, 0);
        }
    }
}