# gl-pic-fluid

*Realtime 3D particle-in-cell fluid simulation*

## Requirements

* OpenGL >= 4.3
* cmake >= 3.10
* C++ compiler for version >= 17

## Building

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. Use generated build system

## Development

* Make sure to list new source files in CMakeLists.txt!
* Simulation code is split across `src/Fluid.hpp` and `*.cs.glsl` shaders in `shader/`
