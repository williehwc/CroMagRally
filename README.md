# Cro-Mag Rally

This fork of CroMagRally replaces OpenGL rendering with TinyGL software rendering. It is intended to be used with [CroMagRallyWatch](https://github.com/williehwc/CroMagRallyWatch), but can also be run as a standalone SDL game on macOS. Linux should also work, but I have not tested it yet. Windows might work, but I am less sure.

## Known issues

- The sky box has been replaced by a 2D scrolling background. The original 3D sky box is warped due to incorrect sampling of textures for faraway polygons.
- The fences disappear temporarily when a lot of polygons are on screen. This issue was especially visible in the Great Wall course, so the dragons have been removed.
- Dirt and smoke particle effects are not visible.
- Skid marks are not visible.

## Build instructions

Follow the build instructions [here](BUILD.md) to build the standalone SDL game on your computer. Build instructions for Apple Watch can be found at [CroMagRallyWatch](https://github.com/williehwc/CroMagRallyWatch).

Cro-Mag Rally © 2000 Pangea Software, Inc. Cro-Mag Rally is a trademark of Pangea Software, Inc.