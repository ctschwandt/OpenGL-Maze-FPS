// Maze.cpp

#include "Maze.h"

#include <algorithm>
#include <climits>
#include <queue>
#include <vector>

bool verbose = false;

//======================================================================
// Maze
//======================================================================

// A dummy cell instance representing walls that shouldn't be punched
namespace
{
    Maze::Cell SENTINEL_INSTANCE(-1, -1, true);
}

// Initialize SENTINEL_CELL to point to the static instance above
Maze::Cell* Maze::SENTINEL_CELL = &SENTINEL_INSTANCE;

// Change in dr and dc used to move
const int Maze::DELTA[4][2] =
{
    { -1,  0 },  // N: up
    {  0,  1 },  // E: right
    {  1,  0 },  // S: down
    {  0, -1 }   // W: left
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
    std::string top    = "+-";
    std::string wall   = "|";
    std::string wumpus = "x";

    for (int r = 0; r < n; ++r)
    {
        // top edges of cells in this row
        for (int c = 0; c < n; ++c)
        {
            int index = r * n + c;
            // seeing if northern wall has been punched
            if (cells[index].neighbors[N] != SENTINEL_CELL
                && cells[index].neighbors[N] != nullptr)
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

        // vertical walls + "wumpus" marker (current path head)
        for (int c = 0; c < n; ++c)
        {
            int index = r * n + c;
            // seeing if western wall has been punched
            if (cells[index].neighbors[W] != SENTINEL_CELL
                && cells[index].neighbors[W] != nullptr)
            {
                wall = " ";
            }
            else
            {
                wall = "|";
            }

            // maze generator character (wumpus)
            if (&cells[index] == path.phead)
                wumpus = "x";
            else
                wumpus = " ";

            std::cout << wall << wumpus;
        }
        std::cout << '|' << std::endl;
    }

    for (int i = 0; i < n; ++i)
        std::cout << "+-";
    std::cout << '+' << std::endl;
}

Maze::Cell & Maze::operator()(int r, int c)
{
    if (r < 0 || r >= n || c < 0 || c >= n)
        throw std::out_of_range("Maze::operator(): invalid row/column index");

    return cells[r * n + c];
}

void Maze::move_once()
{
    int  r = path.top()->r;
    int  c = path.top()->c;
    bool attempted[DIR_SIZE] = { false, false, false, false };

    // check if there is any direction left to try
    auto attempts_left = [](bool attempted[]) -> bool
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
        int dir = std::rand() % DIR_SIZE;
        attempted[dir] = true;

        int new_r = r + DELTA[dir][0];
        int new_c = c + DELTA[dir][1];

        // out of bounds
        if (new_r < 0 || new_r >= n || new_c < 0 || new_c >= n)
        {
            if (verbose)
                std::cout << dir << " is out of bounds\n";
            continue;
        }

        // if already visited
        if ((*this)(new_r, new_c).visited)
        {
            if (verbose)
                std::cout << dir << " already visited\n";
            continue;
        }

        // if a wall (sentinel)
        if ((*this)(r, c).neighbors[dir] == SENTINEL_CELL)
        {
            if (verbose)
                std::cout << dir << " is a wall\n";
            continue;
        }

        if (verbose)
        {
            std::cout << "valid move in " << dir << '\n';
            std::cout << "moved (" << r << ", " << c << ") -> ("
                      << new_r << ", " << new_c << ")\n";
        }

        // Valid move found, so punch a hole through the wall
        (*this)(new_r, new_c).visited = true;

        // Set the neighbor in the current direction
        (*this)(r, c).neighbors[dir] = &(*this)(new_r, new_c);

        // Set the neighbor in the opposite direction for the new cell
        int opposite_dir = (dir + DIR_SIZE / 2) % DIR_SIZE;
        (*this)(new_r, new_c).neighbors[opposite_dir] = &(*this)(r, c);

        // Push the new cell onto the path
        path.push(&(*this)(new_r, new_c));
        return;
    }

    // at this point there are no valid moves, so we backtrack
    if (verbose)
    {
        std::cout << "backtracking from ("
                  << path.top()->r << ", " << path.top()->c << ")\n";
    }

    path.pop();
}

void Maze::init(int start_r, int start_c)
{
    if (n <= 0)
        throw std::runtime_error("Maze::init: maze size n must be > 0");

    if (start_r < 0 || start_r >= n || start_c < 0 || start_c >= n)
        throw std::out_of_range("Maze::init: invalid start cell");

    // tiles_n depends on n; keep it consistent if n was changed externally
    tiles_n = 2 * n + 1;

    // reset all cells
    for (int r = 0; r < n; ++r)
    {
        for (int c = 0; c < n; ++c)
        {
            Cell & cell = cells[r * n + c];
            cell.r       = r;
            cell.c       = c;
            cell.visited = false;
            cell.next    = nullptr;
            for (int i = 0; i < DIR_SIZE; ++i)
                cell.neighbors[i] = nullptr;
        }
    }

    // Starting at the point (start_r, start_c)
    Cell & start = cells[start_r * n + start_c];
    start.visited = true;
    path.phead    = nullptr;
    path.push(&start);

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

    // punching walls until path is empty
    while (path.phead != nullptr)
    {
        move_once();
        if (verbose)
            print();
    }

    // Build precomputed wall tile grid after maze is fully carved
    build_wall_tiles();
}

//----------------------------------------------------------------------
// Path
//----------------------------------------------------------------------

void Maze::Path::pop()
{
    if (phead == nullptr)
        return;

    Cell* q = phead;
    phead   = phead->next;
    q->next = nullptr; // clear link (cells are owned by Maze::cells)
}

void Maze::Path::push(Maze::Cell* cell)
{
    if (!cell) return;
    cell->next = phead;
    phead      = cell;
}

Maze::Cell* Maze::Path::top()
{
    if (phead == nullptr)
        throw std::runtime_error("Path is empty, no head to return.");
    return phead;
}

//----------------------------------------------------------------------
// Wall grid helpers
//----------------------------------------------------------------------

// Compute whether a given tile (tr, tc) is a wall based on cell neighbors.
// This is essentially your old is_wall_tile logic, factored out.
bool Maze::compute_wall_tile(int tr, int tc) const
{
    int tileN = tiles_n;

    // Out of bounds -> treat as wall (for safety)
    if (tr < 0 || tr >= tileN || tc < 0 || tc >= tileN)
        return true;

    // Both odd -> room center (always open floor)
    if (tr % 2 == 1 && tc % 2 == 1)
        return false;

    // Both even -> corner (always wall)
    if (tr % 2 == 0 && tc % 2 == 0)
        return true;

    // Horizontal edge (between two vertical neighbor cells)
    // tr even, tc odd -> between cell (r-1, c) and (r, c)
    if (tr % 2 == 0 && tc % 2 == 1)
    {
        int rBelow = tr / 2;          // cell row below the edge
        int cCell  = (tc - 1) / 2;    // column index of the cells

        // Border edges (outside maze cells) are walls
        if (rBelow <= 0 || rBelow >= n)
            return true;

        int rAbove = rBelow - 1;

        const Cell & above = cells[rAbove * n + cCell];
        const Cell & below = cells[rBelow * n + cCell];

        // If above connects to below, this tile is an opening (no wall)
        if (above.neighbors[S] == &below || below.neighbors[N] == &above)
            return false;

        return true; // no passage -> wall
    }

    // Vertical edge (between two horizontal neighbor cells)
    // tr odd, tc even -> between cell (r, c-1) and (r, c)
    if (tr % 2 == 1 && tc % 2 == 0)
    {
        int cRight = tc / 2;          // cell column to the right
        int rCell  = (tr - 1) / 2;    // cell row

        // Border edges are walls
        if (cRight <= 0 || cRight >= n)
            return true;

        int cLeft = cRight - 1;

        const Cell & left  = cells[rCell * n + cLeft];
        const Cell & right = cells[rCell * n + cRight];

        // If left connects to right, this tile is an opening
        if (left.neighbors[E] == &right || right.neighbors[W] == &left)
            return false;

        return true; // no passage -> wall
    }

    // Fallback (shouldn't happen): be safe and say it's a wall
    return true;
}

// Build the dense wall grid once after generation.
void Maze::build_wall_tiles()
{
    // allocate/resize: 1 byte per tile (0 = open, 1 = wall)
    wall_tiles.assign(tiles_n * tiles_n, 1);

    for (int tr = 0; tr < tiles_n; ++tr)
    {
        for (int tc = 0; tc < tiles_n; ++tc)
        {
            bool w = compute_wall_tile(tr, tc);
            wall_tiles[tr * tiles_n + tc] = static_cast<uint8_t>(w ? 1 : 0);
        }
    }
}

// Fast lookup using precomputed grid.
bool Maze::is_wall_tile(int tr, int tc) const
{
    // Out of bounds -> treat as wall (consistent with old behavior)
    if (tr < 0 || tr >= tiles_n || tc < 0 || tc >= tiles_n)
        return true;

    if (wall_tiles.empty())
    {
        // Safety net: if someone calls this before init(), fall back
        // to computing directly. You could also assert here instead.
        return compute_wall_tile(tr, tc);
    }

    return wall_tiles[tr * tiles_n + tc] != 0;
}

bool Maze::findPath(int startTr, int startTc, int goalTr, int goalTc,
                    std::vector<glm::ivec2> & outPath) const
{
    outPath.clear();
    int tileN = tiles_n;

    if (startTr < 0 || startTr >= tileN || startTc < 0 || startTc >= tileN ||
        goalTr  < 0 || goalTr  >= tileN || goalTc  < 0 || goalTc  >= tileN)
        return false;

    if (is_wall_tile(startTr, startTc) || is_wall_tile(goalTr, goalTc))
        return false;

    if (startTr == goalTr && startTc == goalTc)
    {
        outPath.emplace_back(startTr, startTc);
        return true;
    }

    int totalTiles = tileN * tileN;
    std::vector<int>  gScore(totalTiles, INT_MAX);
    std::vector<int>  parent(totalTiles, -1);
    std::vector<bool> closed(totalTiles, false);

    auto indexOf  = [&](int r, int c) { return r * tileN + c; };
    auto coordsOf = [&](int idx) { return std::pair<int, int>(idx / tileN, idx % tileN); };

    int startIdx = indexOf(startTr, startTc);
    int goalIdx  = indexOf(goalTr, goalTc);
    gScore[startIdx] = 0;

    struct Node
    {
        int idx;
        int f;
    };

    struct CompareNode
    {
        bool operator()(const Node & a, const Node & b) const { return a.f > b.f; }
    };

    std::priority_queue<Node, std::vector<Node>, CompareNode> openSet;

    int initialH = std::abs(goalTr - startTr) + std::abs(goalTc - startTc);
    openSet.push({ startIdx, initialH });

    const int DIRS[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
    bool found           = false;

    while (!openSet.empty())
    {
        Node current = openSet.top();
        openSet.pop();
        int curIdx = current.idx;

        if (closed[curIdx])
            continue;

        closed[curIdx] = true;
        if (curIdx == goalIdx)
        {
            found = true;
            break;
        }

        int cr = curIdx / tileN;
        int cc = curIdx % tileN;

        for (auto & d : DIRS)
        {
            int nr = cr + d[0];
            int nc = cc + d[1];
            if (nr < 0 || nr >= tileN || nc < 0 || nc >= tileN)
                continue;

            if (is_wall_tile(nr, nc))
                continue;

            int neighborIdx = indexOf(nr, nc);
            if (closed[neighborIdx])
                continue;

            int newG = gScore[curIdx] + 1;
            if (newG < gScore[neighborIdx])
            {
                gScore[neighborIdx] = newG;
                parent[neighborIdx] = curIdx;
                int h               = std::abs(goalTr - nr) + std::abs(goalTc - nc);
                openSet.push({ neighborIdx, newG + h });
            }
        }
    }

    if (!found)
        return false;

    std::vector<glm::ivec2> reversePath;
    for (int idx = goalIdx; idx != -1; idx = parent[idx])
    {
        auto [r, c] = coordsOf(idx);
        reversePath.emplace_back(r, c);
        if (idx == startIdx)
            break;
    }

    std::reverse(reversePath.begin(), reversePath.end());
    outPath.swap(reversePath);
    return true;
}

//======================================================================
// Prints (operator<< and debug)
//======================================================================

std::ostream & operator<<(std::ostream & cout, const Maze::Cell & cell)
{
    // simple debug printer; customize if needed
    cout << "Cell(" << cell.r << "," << cell.c << ")";
    return cout;
}

std::ostream & operator<<(std::ostream & cout, const Maze & maze)
{
    // just delegate to print() for now
    cout << "Maze(n=" << maze.n << ")\n";
    return cout;
}

void debug_println(const Maze::Cell & cell)
{
    std::cout << "<Cell: " << &cell << ", "
              << "r: " << cell.r << ", "
              << "c: " << cell.c << ", "
              << "visited: " << cell.visited << ", "
              << "next: " << cell.next << ", "
              << "north: " << cell.neighbors[N] << ", "
              << "east: "  << cell.neighbors[E] << ", "
              << "south: " << cell.neighbors[S] << ", "
              << "west: "  << cell.neighbors[W]
              << '>' << std::endl;
}

void debug_println(const Maze::Path & path)
{
    std::string sep = "    ";
    std::cout << "<Path: " << &path << std::endl;
    Maze::Cell* p = path.phead;
    while (p != nullptr)
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
    for (std::size_t i = 0; i < maze.cells.size(); ++i)
    {
        std::cout << sep;
        debug_println(maze.cells[i]);
    }
    std::cout << '>' << std::endl;
}
