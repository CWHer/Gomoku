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

typedef std::pair<int, int> Pos;
extern int ai_side; //0: black, 1: white
std::string ai_name = "mango";

int turn = 0;
const int N = 15;
const int INF = 1 << 30;
const int dx[] = {0, -1, -1, -1, 0, 1, 1, 1};
const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int values[] = {1, 10, (int)1e3, (int)1e6};

class Box;
class Mango;

//debug begin
std::ofstream fout("mango.out"); //log file
void printpos(Pos pos)
{
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
inline int hamilton(int &x, int &y) //hamilton dist from (7,7) center
{
    return abs(x - 7) + abs(y - 7);
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
    // int calc()    //use after puton
    // {

    // }
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
    void calc() //to be optimized: only calc newly added
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
                            score[board[i][j]] += values[p.first] / 100;
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
    //return 1 if 5 in a row
    bool puton(int x, int y, int col)
    {
        board[x][y] = col;
        hashvalue ^= seed[x][y];
        hashvalue ^= seed[x][y] * (unsigned)(col + 2);
        bbs[x] ^= 1 << y;
        for (int dir = 0; dir < 4; ++dir)
            if (trace(x, y, dir, col).first +
                    trace(x, y, opd(dir), col).first >=
                4)
                return 1;
        return 0;
    }
    void putback(int x, int y)
    {
        bbs[x] ^= 1 << y;
        hashvalue ^= seed[x][y] * (unsigned)(board[x][y] + 2);
        hashvalue ^= seed[x][y];
        board[x][y] = -1;
    }

    unsigned hashresult()
    {
        return hashvalue;
    }

    int getvalue(int col)
    {
        calc();
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
        int ret = bbs[x] << len >> (y - 2) & 31;
        if (x - 1 >= 0)
        {
            ret |= bbs[x - 1] << len >> (y - 2) & 31;
            if (x - 2 >= 0)
                ret |= bbs[x - 2] << len >> (y - 2) & 31;
        }

        if (x + 1 < N)
        {
            ret |= bbs[x + 1] << len >> (y - 2) & 31;
            if (x + 2 < N)
                ret |= bbs[x + 2] << len >> (y - 2) & 31;
        }
        return !ret;
    }
    bool empty33(int x, int y)
    {
        int len = y - 1 >= 0 ? 0 : 1;
        int ret = bbs[x] << len >> (y - 1) & 7;
        if (x - 1 >= 0)
            ret |= bbs[x - 1] << len >> (y - 1) & 7;

        if (x + 1 < N)
            ret |= bbs[x + 1] << len >> (y - 1) & 7;

        return !ret;
    }
};

class Mango
{
private:
    Box box;
    int dfscnt;
    int max_dep;
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
                int col = box.getpos(i, j);
                if (col != -1)
                    continue;

                int sum = 0;
                for (int dir = 0; dir < 8; ++dir)
                {
                    int x = i + dx[dir];
                    int y = j + dy[dir];
                    if (!inboard(i, j))
                    {
                        sum--;
                        continue;
                    }
                    int nxt = box.getpos(i, j);
                    if (nxt == -1)
                        continue;
                    sum += (nxt ^ ai_side) * 3 + 1;
                }

                //debug
                // fout << sum << '\n';

                if (sum > mxval ||
                    (sum == mxval && hamilton(i, j) < hamilton(pos.first, pos.second)))
                {
                    mxval = sum;
                    pos = Pos(i, j);
                }
            }
        box.puton(pos.first, pos.second, ai_side);
        printpos(pos);
        print();
        return pos;
    }

    bool checkswap()
    {
    }

    int mmdfs(int side, int alpha, int beta, int dep, Pos &pos) //min-max
    {
        // if (dep == 5)
        dfscnt++;
        if (dep == max_dep)
            return box.getvalue(ai_side);

        // std::vector<std::pair<Pos, int>> w;
        Pos ret;
        int mxval = -INF;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                int col = box.getpos(i, j);
                if (col != -1)
                    continue;
                if (box.empty33(i, j))
                    continue;

                if (box.puton(i, j, side))
                {
                    if (dep == 1)
                        pos = Pos(i, j);
                    box.putback(i, j);
                    return INF / 2 - dep * 1e7;
                    //win with least steps
                    //while lose with most steps
                }

                int val = -mmdfs(side ^ 1, -beta, -alpha, dep + 1, pos);
                if (val > mxval ||
                    (val == mxval && hamilton(i, j) < hamilton(ret.first, ret.second)))
                {
                    mxval = val, ret = Pos(i, j);
                    if (dep == 1)
                        pos = ret;
                    // fout << val << " pos:" << ret.first << " " << ret.second << '\n';
                }

                // w.push_back(std::pair<Pos, int>(Pos(i, j), -val));
                box.putback(i, j);

                alpha = std::max(alpha, mxval);
                if (mxval > beta)
                    return mxval;
            }
        // std::sort(w.begin(), w.end(),
        //           [](const std::pair<Pos, int> &a,
        //              const std::pair<Pos, int> &b) { return a.second > b.second; });

        return mxval;
    }

    void IDAstar();

public:
    void puton(int x, int y, int col)
    {
        box.puton(x, y, col);
    }
    Pos run()
    {
        // return randput();
        if (turn == 1)
            return randput();
        Pos ret;
        dfscnt = 0;

        // fout << random_Int(0, 2) << '\n';
        max_dep = 5;
        // if (random_Int(0, 2))
        //     max_dep = 3;
        double _t = std::clock();

        mmdfs(ai_side, -INF, INF, 1, ret);

        puton(ret.first, ret.second, ai_side);
        fout << "dfscnt:" << dfscnt << '\n';
        fout << "time:" << (std::clock() - _t) / CLOCKS_PER_SEC << '\n';
        return ret;
    }

    //debug
    void print()
    {
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
    // srand((unsigned)time(0));
    // memset(board, -1, sizeof(board));

    fout << ai_side << std::endl;
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.
// std::pair<int, int> getRandom()
// {
//     while (true)
//     {
//         int x = rand() % 15;
//         int y = rand() % 15;
//         if (board[x][y] == -1)
//         {
//             board[x][y] = ai_side;
//             return std::make_pair(x, y);
//         }
//     }
// }

Mango ai;
Pos action(Pos loc)
{
    /* TODO: Replace this by your code */
    /* This is now a random strategy */
    // ai.print();

    turn++;
    ai.print();
    fout << loc.first << ' ' << loc.second << '\n';
    if (loc.first != -1)
        ai.puton(loc.first, loc.second, ai_side ^ 1);
    ai.print();
    return ai.run();
    // if (turn == 2 && ai_side == 1)
    // {
    //     return std::make_pair(-1, -1);
    // }
    // else
    // {
    //     return ai.run();
    // }
}
