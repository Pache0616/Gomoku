#include "logic.h"

//连续棋子计数器
static int count_continuous(int r, int c, int p, int dr, int dc) {
    int count = 0;
    for (int step = 1; step <= 4; step++) {
        int nr = r + step * dr;
        int nc = c + step * dc;
        if (is_in_board(nr, nc) && board[nr][nc] == p) {
            count++;
        }
        else {
            break;
        }
    }
    return count;
}

//生成五元组查表数据
static int calculate_raw_tuple_score(int b[5]) {
    int b_cnt = 0, w_cnt = 0;
    int mask = 0; 

    // 统计黑白子，并计算二进制掩码
    for (int i = 0; i < 5; i++) {
        if (b[i] == 1) {
            b_cnt++;
            mask |= (1 << (4 - i)); 
        }
        else if (b[i] == 2) {
            w_cnt++;
            mask |= (1 << (4 - i)); 
        }
    }

    //黑白混杂/全空
    if ((b_cnt > 0 && w_cnt > 0) || (b_cnt == 0 && w_cnt == 0)) return 0;

    //打表
    static const int BIT_SCORES[32] = {
        0, 1, 1, 10, 1, 10, 10, 1000, 1, 10, 10, 1000, 10, 1000, 2000, WIN_SCORE,
        1, 10, 10, 1000, 10, 1000, 1000, WIN_SCORE, 10, 1000, 1000, WIN_SCORE, 1000, WIN_SCORE, WIN_SCORE, WIN_SCORE * 100
    };

    return b_cnt > 0 ? BIT_SCORES[mask] : -BIT_SCORES[mask];
}

//提取五元组
static int get_line_score(int r, int c, int dr, int dc) {
    int tuple[5];
    for (int i = 0; i < 5; i++) {
        int nr = r + i * dr, nc = c + i * dc;
        if (!is_in_board(nr, nc)) return 0;
        tuple[i] = board[nr][nc];
    }
    return tuple_score_table[get_tuple_index(tuple)];
}

//判断候选点是否有效,即在5x5范围内是否有棋子
static inline bool has_neighbor(int i, int j) {
    int r_start = (i >= 2) ? i - 2 : 0;
    int r_end = (i + 2 < BOARD_SIZE) ? i + 2 : BOARD_SIZE - 1;
    int c_start = (j >= 2) ? j - 2 : 0;
    int c_end = (j + 2 < BOARD_SIZE) ? j + 2 : BOARD_SIZE - 1;
    for (int r = r_start; r <= r_end; r++) {
        for (int c = c_start; c <= c_end; c++) {
            if (board[r][c] != 0) return true;
        }
    }
    return false;
}

//弃用qsort
static int compare_moves(const void* a, const void* b) {
    return ((Move*)b)->score - ((Move*)a)->score;
}


//打表:生成五元组所有情况的分数表
void init_score_table() {
    int b[5];
    //三进制遍历生成五元组合所有情况 3^5 = 243
    for (int i = 0; i < 243; i++) {
        int temp = i;
        for (int j = 4; j >= 0; j--) {
            b[j] = temp % 3;
            temp /= 3;
        }
        tuple_score_table[i] = calculate_raw_tuple_score(b);
    }
}

//遍历棋盘,计算局势总分
int evaluate_whole_board() {
    int total = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            for (int d = 0; d < 4; d++) {
                int s = get_line_score(i, j, DIRECTIONS[d][0], DIRECTIONS[d][1]);
                if (s >= WIN_SCORE * 100) return WIN_SCORE * 100;
                if (s <= -WIN_SCORE * 100) return -WIN_SCORE * 100;
                total += s;
            }
        }
    }
    return total;
}

//计算单点分数,即落子后该点的局势分数变化
int get_point_score_local(int r, int c) {
    int score = 0;
    for (int d = 0; d < 4; d++) {
        for (int start = -4; start <= 0; start++) {
            int tr = r + start * DIRECTIONS[d][0];
            int tc = c + start * DIRECTIONS[d][1];
            score += abs(get_line_score(tr, tc, DIRECTIONS[d][0], DIRECTIONS[d][1]));
        }
    }
    return score;
}

//判断是否形成五子连珠
bool check_win(int r, int c, int p) {
    for (int i = 0; i < 4; i++) {
        int count = 1; 

        // 正/反向
        count += count_continuous(r, c, p, DIRECTIONS[i][0], DIRECTIONS[i][1]);
        count += count_continuous(r, c, p, -DIRECTIONS[i][0], -DIRECTIONS[i][1]);

        if (count >= 5) return true;
    }
    return false;
}

//生成候选点列表,并按分数排序
int get_candidates(Move* moves) {
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0 && has_neighbor(i, j)) {
                moves[count].r = i;
                moves[count].c = j;
                moves[count].score = get_point_score_local(i, j);
                count++;
            }
        }
    }
    qsort(moves, count, sizeof(Move), compare_moves);
    return count;
}