# Modeler
Goal: A 3D modeling tool built on QT widgets. Still very much a work-in-progress, and with very limited functionality!

# External library dependencies:

- Core: My own custom graphics/rendering library (need to build from source)
- Assimp (Asset Import Library)
- DevIL image library

Dynamic versions of the above libraries are required.

# External asset dependencies:

The application uses some model & texture assets by default, they can be downloaded <a href="projects.markkellogg.org/downloads/Assets.zip">here</a>. Place the uncompressed folder in the build directory for the application.

# Building

It is recommended that you build inside QT Creator. You will need to modify the locations of the Core, Assimp, and DevIL libraries in modeler2.pro. 

