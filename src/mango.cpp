#include "AIController.h"
#include <utility>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <unordered_map>

typedef std::pair<int, int> Pos;
extern int ai_side; //0: black, 1: white
std::string ai_name = "mango";

int turn = 0;
const int N = 15;
const int INF = 1 << 30;
const int dx[] = {0, -1, -1, -1, 0, 1, 1, 1};
const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int values[] = {1, 10, (int)1e3, (int)1e6};
// const int phi = 10;  //first
// const int phi = 100; //second
int phi;

class Box;
class Mango;

//debug begin
const bool _islocal = 1;         //log file control
std::ofstream fout("mango.out"); //log file
void printpos(Pos pos)
{
    if (!_islocal)
        return;
    fout << "pos:" << pos.first << ',' << pos.second << '\n';
}
//debug end

//utility begin
inline bool inboard(int &x, int &y) //inside board
{
    return x >= 0 && x < N && y >= 0 && y < N;
}
inline int opd(int dir) //opposite dir
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
inline int hamilton(int x1, int y1, int x2 = 7, int y2 = 7) //hamilton dist from (7,7) center
{
    return abs(x1 - x2) + abs(y1 - y2);
}
//utility end

class Box //board
{

private:
    unsigned seed[N][N];
    unsigned hashvalue; //condition
    int board[N][N];
    int bbs[N];   //board bit set
    int score[2]; //black/white
    // longest continuous trace
    // plus whether end is blocked
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
    //evaluate all the board
    void calc() // a deserted func, of no use in this version
    {
        score[0] = score[1] = 0;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (board[i][j] != -1)
                {
                    //to avoid multiple calc
                    //only dir in [0,4)
                    //also opposite dir must not be occupied with cur one
                    for (int dir = 0; dir < 4; ++dir)
                    {
                        std::pair<int, bool> p;
                        int x = i + dx[opd(dir)];
                        int y = j + dy[opd(dir)];
                        if (inboard(x, y) && board[x][y] == board[i][j])
                            continue;
                        int blocked = !inboard(x, y) || board[x][y] == (board[i][j] ^ 1);
                        p = trace(i, j, dir, board[i][j]);
                        if (p.second && blocked)
                            continue;
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
    //return 2 when five in a row
    //1 when three in a row without being blocked
    //1 when four in a row
    //default: 0
    int puton(int x, int y, int col)
    {
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
    //check empty in 5*5
    bool empty55(int x, int y)
    {
        int len = y - 2 >= 0 ? 0
                             : y - 1 >= 0 ? 1 : 2;
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
    int check33(int xx, int yy) //a density-biased evaluate func
    {                           //implements like XO chess
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

class Mango
{
private:
    static const int swapdist = 6;
    std::unordered_map<unsigned, int> M;
    Box box;
    int dfscnt;
    int w[N][N];

    //prefer dense places
    //especially the opposite side
    //but it implements like go...
    Pos randput()
    {
        if (turn == 1 && ai_side == 0)
        {
            Pos pos(7, 7);
            puton(7, 7, ai_side);
            return pos;
        }
        Pos pos;
        int mxval = 0;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                if (box.getpos(i, j) != -1)
                    continue;
                int val = box.check33(i, j);
                if (val > mxval)
                {
                    mxval = val;
                    pos = Pos(i, j);
                }
            }
        box.puton(pos.first, pos.second, ai_side);
        printpos(pos);
        print();
        return pos;
    }
    int requiem(int side, int alpha, int beta, int dep, int max_dep, Pos &pos) // final strike
    {
        dfscnt++;
        if (dep == max_dep)
            return box.getvalue(ai_side);
        std::vector<std::pair<Pos, int>> w;
        Pos ret;
        int mxval = -INF;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                if (box.getpos(i, j) != -1)
                    continue;
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
                }
                w.push_back(std::pair<Pos, int>(Pos(i, j), abs(box.getvalue(side) - pre)));
                if (side == ai_side && opt != 1)
                    w.pop_back();
                box.putback(i, j);
            }
        std::sort(w.begin(), w.end(),
                  [](const std::pair<Pos, int> &a,
                     const std::pair<Pos, int> &b) { return a.second > b.second; });
        if (w.empty())
            return box.getvalue(side);
        for (const auto &x : w)
        {
            box.puton(x.first.first, x.first.second, side);
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
                    if (_islocal)
                        fout << mxval << '\n';
                }
            }

            box.putback(x.first.first, x.first.second);

            alpha = std::max(alpha, mxval);
            if (mxval > beta)
                return mxval;
        }
        return mxval;
    }

    int mmdfs(int side, int alpha, int beta, int dep, int max_dep, Pos &pos) //min-max
    {
        dfscnt++;
        if (dep == max_dep)
            return box.getvalue(ai_side);

        std::vector<std::pair<Pos, int>> w;
        Pos ret;
        int mxval = -INF;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                if (box.getpos(i, j) != -1)
                    continue;
                if (box.empty33(i, j))
                    continue;
                int pre = box.getvalue(side);
                if (box.puton(i, j, side) == 2)
                {
                    if (dep == 1)
                        pos = Pos(i, j);
                    box.putback(i, j);
                    return INF / 2 - dep * 1e7;
                    //win with least steps
                    //while lose with most steps
                }
                w.push_back(std::pair<Pos, int>(Pos(i, j), abs(box.getvalue(side) - pre)));
                box.putback(i, j);
            }
        //compromise method
        std::sort(w.begin(), w.end(),
                  [](const std::pair<Pos, int> &a,
                     const std::pair<Pos, int> &b) { return a.second > b.second; });

        //a good but slow method
        // std::sort(w.begin(), w.end(),
        //           [this](const std::pair<Pos, int> &a,
        //                  const std::pair<Pos, int> &b) { return a.second == b.second
        //                                                             ? box.check33(a.first.first, a.first.second) >
        //                                                                   box.check33(b.first.first, b.first.second)
        //                                                             : a.second > b.second; });

        //a awful method
        // std::sort(w.begin(), w.end(),
        //           [](const std::pair<Pos, int> &a,
        //              const std::pair<Pos, int> &b) { return a.second == b.second
        //                                                         ? hamilton(a.first.first, a.first.second) <
        //                                                               hamilton(b.first.first, b.first.second)
        //                                                         : a.second > b.second; });
        if (w.empty())
            return box.getvalue(side);
        for (const auto &x : w)
        {
            box.puton(x.first.first, x.first.second, side);

            int val;
            if (!M.count(box.hashresult()))
            {
                val = -mmdfs(side ^ 1, -beta, -alpha, dep + 1, max_dep, pos);
                M[box.hashresult()] = val;
            }
            else
            {
                val = M[box.hashresult()];
            }

            if (val > mxval ||
                (val == mxval && box.check33(x.first.first, x.first.second) > box.check33(ret.first, ret.second)))
            // (val == mxval && hamilton(x.first.first, x.first.second) > hamilton(ret.first, ret.second)))
            {
                mxval = val, ret = x.first;
                if (dep == 1)
                {
                    pos = ret;
                    printpos(pos);
                    if (_islocal)
                        fout << mxval << '\n';
                }
            }

            box.putback(x.first.first, x.first.second);

            alpha = std::max(alpha, mxval);
            if (mxval > beta)
                return mxval;
        }
        return mxval;
    }

    bool checkswap()
    {
        if (_islocal)
            fout << "swapcheck" << std::endl;
        std::vector<Pos> t(0);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (box.getpos(i, j) == (ai_side ^ 1))
                    t.push_back(Pos(i, j));
        return hamilton(t.front().first, t.front().second,
                        t.back().first, t.back().second) < swapdist;
    }

    Pos IDAstar()
    {
        Pos ret;
        //final strike
        M.clear(), dfscnt = 0;
        if (_islocal)
            fout << "final strike" << '\n';
        if (requiem(ai_side, -INF, INF, 1, 15, ret) > INF / 3)
            return ret;
        if (_islocal)
            fout << "no way" << '\n';
        // check if can win quickly
        for (int max_dep = 3; max_dep <= 7; max_dep += 2)
        {
            if (_islocal)
                fout << "dep:" << max_dep << '\n';
            M.clear();
            if (mmdfs(ai_side, -INF, INF, 1, max_dep, ret) > INF / 3)
                return ret;
        }
        return ret;
    }

public:
    void puton(int x, int y, int col)
    {
        box.puton(x, y, col);
    }
    Pos run()
    {
        if (_islocal)
        {
            fout << "ai_side:" << ai_side << '\n';
            fout << "cur val:" << box.getvalue(ai_side) << '\n';
        }

        // return randput();
        Pos ret;
        if (turn == 1 && ai_side == 0)
            return randput();
        if (turn == 1 && ai_side == 1)
        {
            if (box.getpos(0, 0) != -1)
                ret = Pos(0, N - 1);
            else
                ret = Pos(0, 0);
            puton(ret.first, ret.second, ai_side);
            return ret;
        }
        if (turn == 2 && ai_side == 0)
            return randput();
        double _t = std::clock();
        if (ai_side == 1 && turn == 2)
            if (checkswap())
            {
                ai_side ^= 1;
                return Pos(-1, -1);
            }
        ret = IDAstar();
        puton(ret.first, ret.second, ai_side);
        if (_islocal)
        {
            fout << "dfscnt:" << dfscnt << '\n';
            fout << "time:" << (std::clock() - _t) / CLOCKS_PER_SEC << '\n';
            fout << std::endl;
        }

        return ret;
    }

    //debug
    void print()
    {
        if (!_islocal)
            return;
        fout << "turn: " << turn << '\n';
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int col = box.getpos(i, j);
                fout << (col == -1 ? '.' : col ? '1' : '0');
            }
            fout << '\n';
        }
    }
};

//init function is called once at the beginning
void init()
{
    /* TODO: Replace this by your code */
    phi = ai_side ? 100 : 10;
    if (_islocal)
        fout << ai_side << std::endl;
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.
Mango ai;
Pos action(Pos loc)
{
    turn++;
    ai.print();
    if (_islocal)
        fout << loc.first << ' ' << loc.second << '\n';
    if (loc.first != -1)
        ai.puton(loc.first, loc.second, ai_side ^ 1);
    if (loc.first == -1 && turn > 1)
        ai_side ^= 1;
    ai.print();
    return ai.run();
}
