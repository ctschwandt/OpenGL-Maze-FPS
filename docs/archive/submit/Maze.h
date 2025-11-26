// Maze.h

#ifndef MAZE_H
#define MAZE_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

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
            : r(row), c(col), visited(visit),
              next(NULL), neighbors(new Cell * [DIR_SIZE]())
        {}
        ~Cell()
        {
            if (this != NULL)
            {
                delete [] neighbors;  // Deallocate the neighbors array
            }
        }

        // member variables //
        int r, c;
        bool visited;
        // pointers to neighbors
        Cell * next;
        Cell ** neighbors; // 0N, 1E, 2S, 3W      
    };

    // Used to backtrack steps during maze generation
    class Path
    {
    public:
        Path()
            : phead(NULL)
        {}
        ~Path()
        {}

        // methods //
        void pop();  // delete_head() // return type void?
        void push(Cell *); // insert_head() // return type void?
        Cell * top();// returns head r and c?
        // member variable //
        Cell * phead;
    };

    // Maze components //
    Maze(int size = 0)
        : cells(size * size), path(), n(size)
    {}
    ~Maze()
    {}

    // methods //
    void init(int, int);
    void print();
    Cell & operator()(int, int);
    void move_once();
    
    // member variables //
    std::vector< Cell > cells;
    Path path;
    int n;
    static Cell * SENTINEL_CELL;
    static const int DELTA[4][2];

    // cell print operator<<
    friend std::ostream & operator<<(std::ostream & cout,
                                     const Maze::Cell & cell);
    // maze print operator<<
    friend std::ostream & operator<<(std::ostream & cout,
                                    const Maze & maze);
};


void debug_println(const Maze::Cell &);
void debug_println(const Maze::Path &);
void debug_println(const Maze &);

#endif
