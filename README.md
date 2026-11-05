# Guess The Season Game — The Magic Seasons Adventure

An interactive **C++ computer graphics group project** where players identify the four seasons by exploring animated scenes and choosing the correct season. Built for a Computer Graphics course.

## Features
- Winter, spring, summer, and autumn scenes with season-specific visual elements.
- Character movement with arrow keys, and scene zoom with `+` and `-`.
- Mouse-based season quiz with feedback and progression.
- BMP textures, WAV background audio, and a final 3D rotating trophy.

## Technologies
C++, OpenGL, GLEW, FreeGLUT, Visual Studio (Windows), WinMM `PlaySound`.

## Folder structure
```
src/CGProject.cpp          Main game source
assets/textures/           Five required BMP textures
assets/audio/              Six required WAV sound files
demo/gameplay.mp4          Gameplay demonstration
```

## Build and run (Windows / Visual Studio)
1. Install Visual Studio with **Desktop development with C++** and configure **GLEW** and **FreeGLUT** include/library directories and linker dependencies for your chosen architecture (x86 or x64).
2. Create an **Empty C++ Project**, add `src/CGProject.cpp` as an existing source file, and link the required OpenGL/GLEW/FreeGLUT libraries. The source links `winmm.lib` for audio.
3. Set **Project Properties → Debugging → Working Directory** to the **repository root directory** (the folder containing `assets/`). The code uses relative paths such as `assets\textures\winter.bmp`.
4. Build with **Ctrl+Shift+B** and run with **Ctrl+F5**. Start from the repository root working directory to ensure the textures and audio are found.

> **Note:** This is source code and project assets, not a preconfigured Visual Studio `.sln` project. The exact GLEW/FreeGLUT setup depends on your local Visual Studio installation. The game was successfully developed and tested by the project team. A gameplay demonstration video is available in `demo/gameplay.mp4`. This repository contains the source code and required assets, but does not include a preconfigured Visual Studio `.sln` project.

## Controls
- **PLAY NOW**: start the game.
- **Left/Right arrows**: move the character; on the end screen, rotate the trophy.
- **Up/Down arrows**: move vertically in spring.
- **+ / -**: zoom in/out.
- **Mouse click**: choose the correct season.

## Academic group project
Created collaboratively for a Computer Graphics course. Contributions included graphical scene development, transformations, texture use, testing, and documentation. This repository is a portfolio presentation of the group project, not a claim of sole authorship.

## Asset notes
The original JPEG images supplied for this portfolio were converted to **24-bit BMP** with the dimensions expected by the provided texture loader. Audio files are included as supplied. Confirm rights/permissions before redistributing third-party images or audio publicly.
