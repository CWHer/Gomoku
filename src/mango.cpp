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
#include <iomanip>

typedef std::pair<int, int> Pos;
extern int ai_side; //0: black, 1: white
std::string ai_name = "mango";

int turn = 0;
const int N = 15;
const int INF = 1 << 30;
const int dx[] = {0, -1, -1, -1, 0, 1, 1, 1};
const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

//debug begin
std::ofstream fout("mango.out"); //log file
void printpos(Pos pos)
{
    fout << "pos:" << pos.first << ',' << pos.second << '\n';
}
//debug end

//utility begin
inline bool inboard(int x, int y) //inside board
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
    static std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}
//utility end

class Box //evaluate box
{

private:
    // int type[N][N];
    unsigned seed[N][N];
    unsigned hashvalue; //condition
    int board[N][N];
    int score[2]; //black/white
    // int calc()    //use after puton
    // {

    // }
    inline std::pair<int, bool> trace(int x, int y, int dir, int col) //whether end is blocked
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
    void calc() //evaluate all
    {           //to be optimized: only calc newly added
        score[0] = score[1] = 0;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                if (board[i][j] != -1)
                {
                    //to avoid multiple calc
                    //only dir in [0,4)
                    //also opposite dir must not be placed with cur one
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
                        score[board[i][j]] += 1 << ((p.first + 1 - (p.second | blocked)) << 4);
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
        std::memset(score, 0, sizeof(score));
    }
    void puton(Pos pos, int col)
    {
        board[pos.first][pos.second] = col;
        hashvalue ^= seed[pos.first][pos.second];
        hashvalue ^= seed[pos.first][pos.second] * (unsigned)(col + 2);
    }
    void putback(Pos pos)
    {
        hashvalue ^= seed[pos.first][pos.second] * (unsigned)(board[pos.first][pos.second] + 2);
        hashvalue ^= seed[pos.first][pos.second];
        board[pos.first][pos.second] = -1;
    }
    unsigned hashresult()
    {
        return hashvalue;
    }
    int getvalue(int col)
    {
        return score[col] - score[1 ^ col];
    }
    int getpos(Pos pos)
    {
        return board[pos.first][pos.second];
    }
};

class Mango
{
private:
    Box box;
    int w[N][N];

    //prefer dense places
    //especially the opposite side
    //but it implements like go...
    Pos randput()
    {
        if (turn == 1 && ai_side == 0)
        {
            Pos pos(7, 7);
            puton(pos, ai_side);
            return pos;
        }
        Pos pos;
        int mxval = 0;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                int col = box.getpos(Pos(i, j));
                if (col != -1)
                    continue;

                int sum = 0;
                for (int dir = 0; dir < 8; ++dir)
                {
                    int x = i + dx[dir];
                    int y = j + dy[dir];
                    if (!inboard(x, y))
                    {
                        sum--;
                        continue;
                    }
                    int nxt = box.getpos(Pos(x, y));
                    if (nxt == -1)
                        continue;
                    sum += (nxt ^ ai_side) * 3 + 1;
                }

                //debug
                fout << sum << '\n';

                if (sum > mxval)
                {
                    mxval = sum;
                    pos = Pos(i, j);
                }
            }
        box.puton(pos, ai_side);
        printpos(pos);
        print();
        return pos;
    }

    bool checkswap()
    {
    }

    int mmdfs(int side, int &alpha, int &beta, int dep, Pos &pos) //min-max
    {
        // if (dep == 5)
        if (dep == 3)
            return box.getvalue(ai_side);

        // std::vector<std::pair<Pos, int>> w;
        Pos ret;
        int mxval = -INF;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                int col = box.getpos(Pos(i, j));
                if (col != -1)
                    continue;
                box.puton(Pos(i, j), side);
                int val = -mmdfs(side ^ 1, alpha, beta, dep + 1, pos);
                if (val > mxval)
                {
                    mxval = val, ret = Pos(i, j);
                    fout << val << " pos:" << ret.first << " " << ret.second << '\n';
                }

                // w.push_back(std::pair<Pos, int>(Pos(i, j), -val));
                box.putback(Pos(i, j));
            }
        // std::sort(w.begin(), w.end(),
        //           [](const std::pair<Pos, int> &a,
        //              const std::pair<Pos, int> &b) { return a.second > b.second; });
        if (dep == 1)
        {
            // pos = w.front().first;
            pos = ret;
            return 1; //search complete
        }
        return mxval;
    }

    void IDAstar();

public:
    void puton(Pos pos, int col)
    {
        box.puton(pos, col);
    }
    Pos run()
    {
        // return randput();
        int alpha = -INF, beta = INF;
        Pos ret;
        if (mmdfs(ai_side, alpha, beta, 1, ret))
        {
            puton(ret, ai_side);
            return ret;
        }
    }

    //debug
    void print()
    {
        fout << "turn: " << turn << '\n';
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int col = box.getpos(Pos(i, j));
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
        ai.puton(loc, ai_side ^ 1);
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
