// Maze.cpp

#include "Maze.h"

bool verbose = false;

//======================================================================
// Maze
//======================================================================
// A dummy cell representing walls that shouldn't be punched
Maze::Cell * Maze::SENTINEL_CELL = new Maze::Cell(-1, -1, true);

// Change in dr and dc used to move
const int Maze::DELTA[4][2] =
{
    {-1, 0},   // Move up (north)
    {0, 1},   // Move right (east)
    {1, 0},   // Move down (south)
    {0, -1}  // Move left (west)
};

// prints board
// example board for n = 4:
// +-+-+-+-+
// |   |   |
// +-+ +-+ +
// | |     |
// + +-+-+ +
// |   |   |
// +-+ +-+ +
// |       |
// +-+-+-+-+
void Maze::print()
{      
    std::string top = "+-";
    std::string wall = "|";
    std::string wumpus = "x";
    
    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            int index = r * n + c;
            // seeing if northern wall has beeb punched
            if (cells[index].neighbors[N] != SENTINEL_CELL
                && cells[index].neighbors[N] != NULL)
            {
                top = "+ ";
            }
            else
            {
                top = "+-";
            }
            std::cout << top;
        }
        std::cout << '+' << std::endl;
        
        for (int c = 0; c < n; ++c)
        {
            int index = r * n + c;
            // seeing if western wall has been punched
            if (cells[index].neighbors[W] != SENTINEL_CELL
                && cells[index].neighbors[W] != NULL)
                wall = " ";
            else
                wall = "|";
            // maze generator character (wumpus)
            if (&cells[index] == path.phead)
                wumpus = "x";
            else
                wumpus = " ";
            
            std::cout << wall << wumpus; // cells[r * n + c]; 
        }
        std::cout << '|' << std::endl;
    }
    for (int i = 0; i < n; ++i)
    {
        std::cout << "+-";
    }
    std::cout << '+' << std::endl;
}

Maze::Cell & Maze::operator()(int r, int c)
{
    if (r < 0 || r >= n || c < 0 || c >= n)
        throw std::out_of_range("Invalid row or column index.");
        
    return cells[r * n + c];
}

void Maze::move_once()
{
    int r = path.top()->r;
    int c = path.top()->c;
    bool attempted[DIR_SIZE] = {false, false, false, false};

    // Randomly search for valid attempt
    // while directions are still available
    auto attempts_left = [](bool attempted [])
    {
        for (int i = 0; i < DIR_SIZE; ++i)
        {
            if (!attempted[i])
                return true;
        }
        return false;
    };

    while (attempts_left(attempted))
    {
        int dir = rand() % DIR_SIZE;
        attempted[dir] = true;
        int new_r = r + DELTA[dir][0];
        int new_c = c + DELTA[dir][1];

            // out of bounds
            if (new_r < 0 || new_r >= n || new_c < 0 || new_c >= n)
            {
                if (verbose)
                    std::cout << dir << " is out of bounds " << std::endl;
                continue;
            }
            // if already visited
            if ((*this)(new_r, new_c).visited)
            {
                if (verbose)
                    std::cout << dir << " already visited " << std::endl;
                continue;
            }
            // if a wall
            if ((*this)(r, c).neighbors[dir] == SENTINEL_CELL)
            {
                if (verbose)
                    std::cout << dir << " is a wall " << std::endl;
                continue;
            }

            if (verbose)
            {
                std::cout << "valid move in " << dir << std::endl;
                std::cout << "moved " << '(' << r << ", " << c << ')'
                          << " to " << '(' << new_r << ", " << new_c << ')'
                          << std::endl;
            }
        // Valid move found, so punch a hole through the wall
        ((*this)(new_r, new_c)).visited = true; 
        // Set the neighbor in the current direction
        ((*this)(r, c)).neighbors[dir] = &((*this)(new_r, new_c));
        // Set the neighbor in the opposite direction for the new cell
        int opposite_dir = (dir + DIR_SIZE / 2) % DIR_SIZE;
        ((*this)(new_r, new_c)).neighbors[opposite_dir] = &((*this)(r, c));
        // Push the new cell onto the path
        path.push(&(*this)(new_r, new_c));
        return;
    }

    // at this point there are no valid moves, so we backtrack
    if (verbose)
    {
        std::cout << "backtracking from "
                  << '(' << path.top()->r << ", " << path.top()->c << ')'
              << std::endl;
    }
    
    path.pop();
    
    return;
}

void Maze::init(int r, int c)
{   
    // Starting at the point (r, c)
    path.push(&cells[r * n + c]);
    cells[r * n + c].visited = true;

    // Setting cells to their coordinates
    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            cells[r * n + c].r = r;
            cells[r * n + c].c = c;
        }
    }

    // Setting perimeter walls to sentinel node
    for (int c = 0; c < n; ++c)
    {
        // Northern wall (first row)
        cells[c].neighbors[N] = SENTINEL_CELL;
        // Southern wall (last row)
        cells[(n - 1) * n + c].neighbors[S] = SENTINEL_CELL;
    }
    for (int r = 0; r < n; ++r)
    {
        // Western wall (first column of each row)
        cells[r * n].neighbors[W] = SENTINEL_CELL;
        // Eastern wall (last column of each row)
        cells[r * n + (n - 1)].neighbors[E] = SENTINEL_CELL;
    }

    // punching walls
    while (path.phead != NULL)
    {
        move_once();
        if (verbose)
            print();
    }
}

//----------------------------------------------------------------------
// Cell
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Path
//----------------------------------------------------------------------
void Maze::Path::pop()
{
    if (phead == NULL)
        return;
    
    Cell * q = phead;
    phead = phead->next;
}

void Maze::Path::push(Maze::Cell * cell)
{
    cell->next = phead;
    phead = cell;
}

Maze::Cell * Maze::Path::top()
{
    if (phead == NULL)
    {
        throw std::runtime_error("Path is empty, no head to return.");
    }
    
    return phead;
}

//======================================================================
// Prints
//======================================================================
std::ostream & operator<<(std::ostream & cout, const Maze::Cell & cell)
{
    return cout;
}

std::ostream & operator<<(std::ostream & cout, const Maze & maze)
{
    return cout;
}

void debug_println(const Maze::Cell & cell)
{
    std::cout << "<Cell: " << &cell << ", "
              << "r: " << cell.r << ", "
              << "c: " << cell.c << ", "  
              << "next: " << cell.next << ", "
              << "north: " << cell.neighbors[0] << ", "
              << "east: " << cell.neighbors[1] << ", "
              << "south: " << cell.neighbors[2] << ", "
              << "west: " << cell.neighbors[3]
              << '>'
              << std::endl;
}

void debug_println(const Maze::Path & path)
{
    std::string sep = "    ";
    std::cout << "<Path: " << &path << std::endl;
    Maze::Cell * p = path.phead;
    while (p != NULL)
    {
        std::cout << sep;
        debug_println(*p);
        p = p->next; 
    }
    std::cout << '>' << std::endl;
}

void debug_println(const Maze & maze)
{
    std::string sep = "    ";
    std::cout << "SENTINEL_CELL = " << Maze::SENTINEL_CELL << std::endl;
    std::cout << "<Maze: " << &maze << std::endl;
    for (int i = 0; i < maze.cells.size(); ++i)
    {
        std::cout << sep;
        debug_println(maze.cells[i]);
    }
    std::cout << '>' << std::endl;  
}
