// #include <utility>
#include <iostream>
#include <random>
#include <ctime>
#include <cstring>
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
ofstream fout("test.out");

//utility begin
inline bool inboard(int &x, int &y) //inside board
{
    return x >= 0 && x < N && y >= 0 && y < N;
}
inline int opd(int &dir) //opposite dir
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
void printpos(Pos pos)
{
    std::cout << "pos:" << pos.first << ',' << pos.second << std ::endl;
}
//utility end

class Box //evaluate box
{
public:
    unsigned seed[N][N];
    unsigned hashvalue; //condition
    int board[N][N];
    int bbs[N];   //board bit set
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
        // fprint();
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
                            score[board[i][j]] += values[p.first] / 2;
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
    void read()
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
            {
                char ch = getchar();
                while (ch == '\n')
                    ch = getchar();
                if (ch != '.')
                    board[i][j] = ch - '0', bbs[i] ^= 1 << j;
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
    void print()
    {
        cout << "turn: " << turn << '\n';
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int col = getpos(i, j);
                cout << (col == -1 ? '.' : col ? '1' : '0');
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
                fout << (col == -1 ? '.' : col ? '1' : '0');
            }
            fout << '\n';
        }
    }
};

Box box;
int dfscnt = 0;
int mmdfs(int side, int alpha, int beta, int dep, Pos &pos) //min-max
{
    // if (dep == 5)
    dfscnt++;
    if (dep == 5)
    {
        // box.fprint();
        // fout << box.getvalue(ai_side) << '\n';
        return box.getvalue(ai_side);
    }

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

            if (box.rempty33(i, j) != box.empty33(i, j))
            {
                box.print();
                cout << box.rempty33(i, j) << endl;
                cout << box.empty33(i, j) << endl;
            }

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
                {
                    pos = ret;
                    printpos(pos);
                    cout << mxval << endl;
                    putchar('\n');
                }

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
int main()
{
    double t = clock();

    freopen("in", "r", stdin);
    ai_side = 1;
    // box.board[0][0] = box.board[0][1] = 1;
    // box.board[3][0] = box.board[3][1] = box.board[3][2] = 0;
    box.read();
    box.print();
    std::cout << box.getvalue(1) << std::endl;
    Pos pos;
    mmdfs(ai_side, -INF, INF, 1, pos);
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