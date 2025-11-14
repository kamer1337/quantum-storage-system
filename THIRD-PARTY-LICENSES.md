# Third-Party Licenses

This project includes or depends on the following third-party software:

## GLFW

**Version**: 3.3.8 (bundled in glfw/glfw-master)  
**License**: zlib/libpng license  
**Website**: https://www.glfw.org/  
**Repository**: https://github.com/glfw/glfw

GLFW is used for windowing and input handling in the GUI mode.

The full GLFW license can be found in: `glfw/glfw-master/LICENSE.md`

### GLFW License Summary

Copyright © 2002-2006 Marcus Geelnard  
Copyright © 2006-2019 Camilla Löwy

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

## OpenGL

OpenGL is not bundled with this software. Users must have OpenGL 2.1+ compatible drivers installed on their system. OpenGL is typically provided by graphics card vendors.

**Note**: The Quantum Storage System uses the OpenGL API but does not include any OpenGL libraries. The system requires OpenGL libraries to be available on the target system.

## API GUI (Pure C 5D Renderer)

The API GUI rendering system is a custom-built, pure C implementation developed specifically for this project. It has no external dependencies except GLFW (for windowing) and OpenGL (for rendering).

**License**: Same as the main project (MIT License)  
**Location**: `src/gui/api_gui.c` and `src/gui/api_gui.h`

---

For the main project license, see the LICENSE file in the root directory.
