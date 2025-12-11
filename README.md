# OpenGL Maze FPS

A 3D first-person shooter that uses OpenGL with GLFW for windowing/input to render randomly generated maze environments. The project features dual camera views (first-person and bird's-eye), textured walls/objects, moving robot enemies, shooting mechanics, and basic collision detection.

## Repository structure

- `src/` – Source files for the game logic and rendering pipeline.
  - `src/mygllib/` – OpenGL helper implementations (view handling, window config, singletons).
- `include/` – Project headers shared across source files.
  - `include/mygllib/` – Helper headers for camera/view management and related utilities.
- `assets/` – Artifacts such as textures, models, and shader programs.
  - `assets/shaders/` – GLSL shaders for rendering.
  - `assets/textures/` – Texture assets for maze walls and objects.
  - `assets/models/` – Static models used in the scene.
- `docs/` – Project documentation, including the original project write-up (`OpenGL-Maze-FPS.pdf`) and legacy submission artifacts under `docs/archive/`.

## Getting started

1. Ensure you have a modern C++ compiler along with OpenGL, GLFW, and GLEW installed.
2. Build the project from the repository root using the provided `Makefile`:

   ```sh
   make
   ```

3. Run the compiled binary:

   ```sh
   make run
   ```

4. Add any required textures, shaders, and models inside the corresponding `assets/` subdirectories.

The updated layout keeps assets, headers, documentation, and build artifacts neatly separated, making the project easier to navigate and extend.
