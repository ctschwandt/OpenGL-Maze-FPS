// Maze.h

#ifndef MAZE_H
#define MAZE_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stdexcept>

// directions North, East, South, West
enum DIR
{
    N, E, S, W, DIR_SIZE
};

class Maze
{
public:
    // internal classes //
    class Cell;
    class Path;

    // A cell within the maze
    class Cell
    {
    public:
        Cell(int row = 0, int col = 0, bool visit = false)
            : r(row),
              c(col),
              visited(visit),
              next(nullptr)
        {
            for (int i = 0; i < DIR_SIZE; ++i)
                neighbors[i] = nullptr;
        }

        // member variables //
        int   r, c;
        bool  visited;
        // pointers to neighbors
        Cell* next;
        Cell* neighbors[DIR_SIZE]; // 0N, 1E, 2S, 3W
    };

    // Used to backtrack steps during maze generation
    class Path
    {
    public:
        Path()
            : phead(nullptr)
        {}
        ~Path() = default;

        // methods //
        void  pop();           // delete head
        void  push(Cell*);     // insert head
        Cell* top();           // returns head

        // member variable //
        Cell* phead;
    };

    // Maze components //
    Maze(int size = 0, int r = 0, int c = 0)
        : cells(size * size),
          path(),
          n(size),
          tiles_n(2 * n + 1)
    {}

    ~Maze() = default;

    // methods //
    void   init(int start_r, int start_c);
    void   print();
    Cell & operator()(int r, int c);
    void   move_once();
    bool is_wall_tile(int tr, int tc) const;
    
    // member variables //
    std::vector<Cell> cells;
    Path              path;
    int               n;
    int               tiles_n;

    static Cell*      SENTINEL_CELL;
    static const int  DELTA[4][2];

    // cell print operator<<
    friend std::ostream & operator<<(std::ostream & cout,
                                     const Maze::Cell & cell);
    // maze print operator<<
    friend std::ostream & operator<<(std::ostream & cout,
                                     const Maze & maze);
};


// debug helpers
void debug_println(const Maze::Cell &);
void debug_println(const Maze::Path &);
void debug_println(const Maze &);

#endif // MAZE_H
