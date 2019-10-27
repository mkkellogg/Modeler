# Modeler
Goal: A 3D scene staging tool and physically-based rendering sandbox built on QT widgets. Makes use of my own custom rendering engine (Core), and serves as a great utility to test ongoing feature development in the engine. Still very much a work-in-progress!

## Current functionality:
A built-in scene that showcases the current state of the physically-based rendering capabilities of Core will be loaded by default. Additional models can be imported into the scene and repositioned. Modifiable model import settings include scale, normal smoothing threshold, and material type (physically-based or "legacy"). 

## External library dependencies:

- Core: My own custom graphics/rendering library (need to build from source)
- Assimp (Asset Import Library)
- DevIL image library

Dynamic versions of the above libraries are required.

## External asset dependencies:

The application uses some model & texture assets by default, they can be downloaded <a href="http://projects.markkellogg.org/downloads/assets.zip">here</a>. Extract the contents and place them in a folder called `assets` in the build directory for the application.

## Building

It is recommended that you build inside QT Creator. You will need to modify the locations of the Core, Assimp, and DevIL libraries in modeler2.pro. This can be done by editing the following variables: CORE_BUILD_DIR, ASSIMP_INCLUDE_DIR, ASSIMP_BUILD_DIR, and DEVIL_BUILD_DIR.

