// #include <utility>
#include <iostream>
#include <random>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
using namespace std;

typedef std::pair<int, int> Pos;
int turn = 0;
int ai_side;
const int N = 15;
const int INF = 1 << 30;
const int dx[] = {0, -1, -1, -1, 0, 1, 1, 1};
const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int values[] = {1, 10, (int)1e3, (int)1e6};
const int phi = 50;
// unordered_set<unsigned> S;
unordered_map<unsigned, int> M;
ofstream fout("test.out");

// utility begin
inline bool inboard(int &x, int &y) // inside board
{
    return x >= 0 && x < N && y >= 0 && y < N;
}
inline int opd(int dir) // opposite dir
{
    return (dir + 4) % 8;
}
inline int random_int(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}
inline int hamilton(int x1, int y1, int x2 = 7, int y2 = 7) // hamilton dist from (7,7) center
{
    return abs(x1 - x2) + abs(y1 - y2);
}
void printpos(Pos pos)
{
    std::cout << "pos:" << pos.first << ',' << pos.second << std ::endl;
}
// utility end

class Box // evaluate box
{
public:
    unsigned seed[N][N];
    unsigned hashvalue; // condition
    int board[N][N];
    int bbs[N];   // board bit set
    int score[2]; // black/white
    inline std::pair<int, bool> trace(int x, int y, int dir, int col)
    {
        int ret = 0;
        while (true)
        {
            x += dx[dir], y += dy[dir];
            if (!inboard(x, y) || board[x][y] == (col ^ 1))
                return std::pair<int, bool>(ret, 1);
            if (board[x][y] == -1)
                return std::pair<int, bool>(ret, 0);
            ret++;
        }
    }
    void calc() // evaluate all
    {           // to be optimized: only calc newly added
        score[0] = score[1] = 0;
        // fprint();
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (board[i][j] != -1)
                {
                    // to avoid multiple calc
                    // only dir in [0,4)
                    // also opposite dir must not be placed with cur one
                    for (int dir = 0; dir < 4; ++dir)
                    {
                        std::pair<int, bool> p;
                        int x = i + dx[opd(dir)];
                        int y = j + dy[opd(dir)];
                        if (inboard(x, y) && board[x][y] == board[i][j])
                            continue;
                        int blocked = !inboard(x, y) || board[x][y] == (board[i][j] ^ 1);
                        p = trace(i, j, dir, board[i][j]);
                        // if (p.first == 4)
                        // {
                        //     fprint();
                        //     fout << "!" << endl;
                        // }
                        if (p.second && blocked)
                            continue;
                        // score[board[i][j]] += 1 << ((p.first + 1 - (p.second | blocked)) << 2);
                        // if (p.first + 1 - (p.second | blocked) == 4)
                        // {
                        //     putchar('!');
                        // }
                        // p.second = blocked = 0;
                        if (p.second || blocked)
                            score[board[i][j]] += values[p.first] / phi;
                        else
                            score[board[i][j]] += values[p.first];
                    }
                }
    }

public:
    Box()
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                seed[i][j] = random_int(0, INT32_MAX);
                hashvalue ^= seed[i][j];
            }

        std::memset(board, -1, sizeof(board));
        std::memset(bbs, 0, sizeof(bbs));
        std::memset(score, 0, sizeof(score));
    }
    // return 2 when five in a row
    // 1 when three in a row without being blocked
    // 1 when four in a row
    // default: 0
    int puton(int x, int y, int col)
    {
        // if (x == 2 && y == 0 && col == 0)
        // {
        //     putchar('\n');
        // }

        board[x][y] = col;
        hashvalue ^= seed[x][y];
        hashvalue ^= seed[x][y] * (unsigned)(col + 2);
        bbs[x] ^= 1 << y;
        int ret = 0;
        for (int dir = 0; dir < 4; ++dir)
        {
            std::pair<int, bool> ls[2], rs[2];
            for (int k = 0; k < 2; ++k)
            {
                ls[k] = trace(x, y, dir, k);
                rs[k] = trace(x, y, opd(dir), k);
            }
            // 5 in a row!
            if (ls[col].first + rs[col].first >= 4)
                return 2;
            // stop
            if (ls[col ^ 1].first)
            {
                int len = ls[col ^ 1].first - 1;
                if (ls[col ^ 1].second)
                    score[col ^ 1] -= values[len] / phi;
                else
                    score[col ^ 1] += values[len] / phi - values[len];
            }
            if (rs[col ^ 1].first)
            {
                int len = rs[col ^ 1].first - 1;
                if (rs[col ^ 1].second)
                    score[col ^ 1] -= values[len] / phi;
                else
                    score[col ^ 1] += values[len] / phi - values[len];
            }
            // join
            if (ls[col].first)
            {
                int len = ls[col].first - 1;
                if (ls[col].second)
                    score[col] -= values[len] / phi;
                else
                    score[col] -= values[len];
            }
            if (rs[col].first)
            {
                int len = rs[col].first - 1;
                if (rs[col].second)
                    score[col] -= values[len] / phi;
                else
                    score[col] -= values[len];
            }
            if (ls[col].second && rs[col].second)
                continue;
            if (ls[col].second || rs[col].second)
            {
                score[col] += values[ls[col].first + rs[col].first] / phi;
                if (ls[col].first + rs[col].first == 3)
                    ret = 1;
            }

            else
            {
                score[col] += values[ls[col].first + rs[col].first];
                if (ls[col].first + rs[col].first >= 2)
                    ret = 1;
            }
        }
        return ret;
    }
    void putback(int x, int y)
    {
        bbs[x] ^= 1 << y;
        hashvalue ^= seed[x][y] * (unsigned)(board[x][y] + 2);
        hashvalue ^= seed[x][y];
        int col = board[x][y];
        board[x][y] = -1;
        for (int dir = 0; dir < 4; ++dir)
        {
            std::pair<int, bool> ls[2], rs[2];
            for (int k = 0; k < 2; ++k)
            {
                ls[k] = trace(x, y, dir, k);
                rs[k] = trace(x, y, opd(dir), k);
            }
            // 5 in a row!
            if (ls[col].first + rs[col].first >= 4)
                return;
            // stop
            if (ls[col ^ 1].first)
            {
                int len = ls[col ^ 1].first - 1;
                if (ls[col ^ 1].second)
                    score[col ^ 1] += values[len] / phi;
                else
                    score[col ^ 1] -= values[len] / phi - values[len];
            }
            if (rs[col ^ 1].first)
            {
                int len = rs[col ^ 1].first - 1;
                if (rs[col ^ 1].second)
                    score[col ^ 1] += values[len] / phi;
                else
                    score[col ^ 1] -= values[len] / phi - values[len];
            }
            // join
            if (ls[col].first)
            {
                int len = ls[col].first - 1;
                if (ls[col].second)
                    score[col] += values[len] / phi;
                else
                    score[col] += values[len];
            }
            if (rs[col].first)
            {
                int len = rs[col].first - 1;
                if (rs[col].second)
                    score[col] += values[len] / phi;
                else
                    score[col] += values[len];
            }
            if (ls[col].second && rs[col].second)
                continue;
            if (ls[col].second || rs[col].second)
                score[col] -= values[ls[col].first + rs[col].first] / phi;
            else
                score[col] -= values[ls[col].first + rs[col].first];
        }
    }
    unsigned hashresult()
    {
        return hashvalue;
    }
    int getvalue(int col)
    {
        // calc();
        return score[col] - score[1 ^ col];
    }
    int getpos(int x, int y)
    {
        return board[x][y];
    }
    void read()
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                char ch = getchar();
                while (ch == '\n')
                    ch = getchar();
                if (ch != '.')
                    puton(i, j, ch - '0');
                // board[i][j] = ch - '0', bbs[i] ^= 1 << j;
            }
    }
    // check empty in 5*5
    bool rempty33(int x, int y)
    {
        for (int i = x - 1; i <= x + 1; ++i)
            for (int j = y - 1; j <= y + 1; ++j)
                if (inboard(i, j) && getpos(i, j) != -1)
                    return 0;
        return 1;
    }
    bool empty55(int x, int y)
    {
        int len = y - 2 >= 0   ? 0
                  : y - 1 >= 0 ? 1
                               : 2;
        int p = y - 2 + len;
        int ret = bbs[x] << len >> p & 31;
        if (x - 1 >= 0)
        {
            ret |= bbs[x - 1] << len >> p & 31;
            if (x - 2 >= 0)
                ret |= bbs[x - 2] << len >> p & 31;
        }

        if (x + 1 < N)
        {
            ret |= bbs[x + 1] << len >> p & 31;
            if (x + 2 < N)
                ret |= bbs[x + 2] << len >> p & 31;
        }
        return !ret;
    }
    bool empty33(int x, int y)
    {
        int len = y - 1 >= 0 ? 0 : 1;
        int p = y - 1 + len;
        int ret = bbs[x] << len >> p & 7;
        if (x - 1 >= 0)
            ret |= bbs[x - 1] << len >> p & 7;

        if (x + 1 < N)
            ret |= bbs[x + 1] << len >> p & 7;

        return !ret;
    }
    void print()
    {
        cout << "turn: " << turn << '\n';
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int col = getpos(i, j);
                cout << (col == -1 ? '.' : col ? '1'
                                               : '0');
            }
            cout << '\n';
        }
    }
    void fprint()
    {
        fout << "turn: " << turn << '\n';
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int col = getpos(i, j);
                fout << (col == -1 ? '.' : col ? '1'
                                               : '0');
            }
            fout << '\n';
        }
    }
    int check33(int xx, int yy) // a density-biased evaluate func
    {                           // implements like XO chess
        int ret = 0;
        for (int dir = 0; dir < 8; ++dir)
        {
            int x = xx + dx[dir];
            int y = yy + dy[dir];
            if (!inboard(x, y))
            {
                ret--;
                continue;
            }
            int nxt = getpos(x, y);
            if (nxt != -1)
                ret += (nxt ^ ai_side) * 2 + 1;
        }
        return ret;
    }
};

Box box;
int dfscnt = 0;
int max_dep;
int requiem(int side, int alpha, int beta, int dep, int max_dep, Pos &pos) // final strike
{
    dfscnt++;
    if (dep == max_dep)
    {
        // box.fprint();
        // fout << box.getvalue(ai_side) << '\n';
        return box.getvalue(ai_side);
    }

    std::vector<std::pair<Pos, int>> w;
    Pos ret;
    int mxval = -INF;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            if (box.getpos(i, j) != -1)
                continue;
            // assert(box.rempty33(i, j) == box.empty33(i, j));
            if (box.empty33(i, j))
                continue;
            int pre = box.getvalue(side);
            int opt = box.puton(i, j, side);
            if (opt == 2)
            {
                if (dep == 1)
                    pos = Pos(i, j);
                box.putback(i, j);
                return INF / 2 - dep * 1e7;
                // win with least steps
                // while lose with most steps
            }
            w.push_back(std::pair<Pos, int>(Pos(i, j), abs(box.getvalue(side) - pre)));
            if (side == ai_side && opt != 1)
                w.pop_back();

            box.putback(i, j);
        }
    std::sort(w.begin(), w.end(),
              [](const std::pair<Pos, int> &a,
                 const std::pair<Pos, int> &b)
              { return a.second > b.second; });
    if (w.empty())
        return box.getvalue(side);
    for (const auto &x : w)
    {
        box.puton(x.first.first, x.first.second, side);
        // S.insert(box.hashresult());
        int val;
        if (!M.count(box.hashresult()))
        {
            val = -requiem(side ^ 1, -beta, -alpha, dep + 1, max_dep, pos);
            M[box.hashresult()] = val;
        }
        else
        {
            val = M[box.hashresult()];
        }

        if (val > mxval ||
            (val == mxval && box.check33(x.first.first, x.first.second) > box.check33(ret.first, ret.second)))
        {
            mxval = val, ret = x.first;
            if (dep == 1)
            {
                pos = ret;
                printpos(pos);
                cout << mxval << endl;
                putchar('\n');
            }

            // fout << val << " pos:" << ret.first << " " << ret.second << '\n';
        }

        box.putback(x.first.first, x.first.second);

        alpha = std::max(alpha, mxval);
        if (mxval > beta)
            return mxval;
    }
    return mxval;
}
int mmdfs(int side, int alpha, int beta, int dep, Pos &pos) // min-max
{
    // if (dep == 5)
    dfscnt++;
    if (dep == max_dep)
    {
        // box.fprint();
        // fout << box.getvalue(ai_side) << '\n';
        return box.getvalue(ai_side);
    }

    std::vector<std::pair<Pos, int>> w;
    Pos ret;
    int mxval = -INF;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            if (box.getpos(i, j) != -1)
                continue;
            // assert(box.rempty33(i, j) == box.empty33(i, j));
            if (box.empty33(i, j))
                continue;
            int pre = box.getvalue(side);
            if (box.puton(i, j, side) == 2)
            {
                if (dep == 1)
                    pos = Pos(i, j);
                box.putback(i, j);
                return INF / 2 - dep * 1e7;
                // win with least steps
                // while lose with most steps
            }
            w.push_back(std::pair<Pos, int>(Pos(i, j), abs(box.getvalue(side) - pre)));
            box.putback(i, j);
        }
    std::sort(w.begin(), w.end(),
              [](const std::pair<Pos, int> &a,
                 const std::pair<Pos, int> &b)
              { return a.second == b.second
                           ? hamilton(a.first.first, a.first.second) <
                                 hamilton(b.first.first, b.first.second)
                           : a.second > b.second; });
    if (w.empty())
        return box.getvalue(side);
    for (const auto &x : w)
    {
        box.puton(x.first.first, x.first.second, side);
        // S.insert(box.hashresult());
        int val;
        if (!M.count(box.hashresult()))
        {
            val = -mmdfs(side ^ 1, -beta, -alpha, dep + 1, pos);
            M[box.hashresult()] = val;
        }
        else
        {
            val = M[box.hashresult()];
        }

        if (val > mxval ||
            (val == mxval && box.check33(x.first.first, x.first.second) > box.check33(ret.first, ret.second)))
        {
            mxval = val, ret = x.first;
            if (dep == 1)
            {
                pos = ret;
                printpos(pos);
                cout << mxval << endl;
                putchar('\n');
            }

            // fout << val << " pos:" << ret.first << " " << ret.second << '\n';
        }

        box.putback(x.first.first, x.first.second);

        alpha = std::max(alpha, mxval);
        if (mxval > beta)
            return mxval;
    }
    return mxval;
}

Pos checkswap()
{
    fout << "swapcheck" << std::endl;
    double _t = std::clock();
    Pos ret;
    int val, nval; // not sawp val
    val = mmdfs(ai_side ^ 1, -INF, INF, 1, ret);
    nval = mmdfs(ai_side, -INF, INF, 1, ret);
    if (val < nval)
        ret = Pos(-1, -1);
    fout << "dfscnt:" << dfscnt << '\n';
    fout << "time:" << (std::clock() - _t) / CLOCKS_PER_SEC << '\n';
}

int main()
{
    double t = clock();

    freopen("in", "r", stdin);
    ai_side = 1;
    // box.board[0][0] = box.board[0][1] = 1;
    // box.board[3][0] = box.board[3][1] = box.board[3][2] = 0;
    box.read();
    box.print();
    std::cout << box.getvalue(ai_side) << std::endl;
    Pos pos;
    max_dep = 9;
    // S.clear();
    M.clear();
    mmdfs(ai_side, -INF, INF, 1, pos);
    // requiem(ai_side, -INF, INF, 1, 15, pos);
    printpos(pos);

    cout << dfscnt << endl;
    cout << (clock() - t) / CLOCKS_PER_SEC << '\n';

    // box.puton(Pos(1, 8), ai_side);
    // cout << box.getvalue(1) << endl;
    // box.putback(Pos(1, 8));

    // box.puton(Pos(2, 8), ai_side);
    // cout << box.getvalue(1) << endl;
    // box.putback(Pos(2, 8));
    // box.read();
    // cout << box.getvalue(1) << endl;
    return 0;
}