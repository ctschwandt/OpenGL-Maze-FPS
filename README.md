# OpenGL Maze FPS (in progress)

A 3D first-person shooter that uses OpenGL and FreeGLUT to render randomly generated maze environments. The project features dual camera views (first-person and bird's-eye), textured walls/objects, moving robot enemies, shooting mechanics, and basic collision detection.

## Repository structure

- `src/` – Source files for the game logic and rendering pipeline.
- `include/` – Project headers shared across source files.
- `assets/` – Artifacts such as textures, models, and shader programs.
  - `assets/shaders/` – GLSL shaders for rendering.
  - `assets/textures/` – Texture assets for maze walls and objects.
  - `assets/models/` – Static models used in the scene.
- `docs/` – Project documentation, including the original project write-up (`OpenGL-Maze-FPS.pdf`).

## Getting started

1. Ensure you have a modern C++ compiler, CMake, OpenGL, and FreeGLUT installed.
2. Add your source files to `src/` and headers to `include/`.
3. Place any required textures, shaders, and models inside the corresponding `assets/` subdirectories.
4. Configure your preferred build system (e.g., CMake or Make) to reference the organized directories above.

This structure keeps assets and documentation separate from source code, making the project easier to navigate and extend.
