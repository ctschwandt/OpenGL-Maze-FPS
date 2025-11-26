// Name: Cole Schwandt
// File: main.cpp
//
// Description:
// Horrendously programmed maze generator

#include <iostream>
#include "Maze.h"

int main()
{
    srand((unsigned int) time(NULL));
    
    // Maze is in a grid of n-by-n cells. The starting point is (r, c).
    int n, r, c;
    std::cin >> n >> r >> c;
    
    Maze maze(n);
    maze.init(r, c);
    maze.print();
    
    return 0;
}
