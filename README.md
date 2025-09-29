# OpenGL Maze FPS

A 3D first-person shooter built with **C++**, **OpenGL**, and **FreeGLUT**.  

## Features
- Procedurally generated maze environments for unique layouts each run
- Textured walls and objects for improved visual detail
- Dual camera views: immersive first-person and overhead bird’s-eye
- Shooting mechanics with moving robot enemies
- Collision detection for walls, bullets, and objects
- Optimized rendering with visibility checks to only draw entities within the player’s view for smoother performance

## Build
Make sure you have OpenGL and FreeGLUT installed. Then build with:
\`\`\`bash
make
\`\`\`

## Run
\`\`\`bash
./a.out
\`\`\`

Use keyboard and mouse controls to move, explore the maze, and engage with enemies.  

---

## Project Structure
- main.cpp — entry point and game loop  
- makefile — build instructions  
- *.h / *.cpp — rendering, input handling, collision, and entity logic  

---

## Future Improvements
- Add sound effects and background music  
- Expand enemy AI behavior  
- Implement scoring system and levels  
- Add pause menu and HUD
