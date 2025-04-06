# ZHenerals

A reimplementation to play _Command & Conquer: Generals Zero Hour_.

## Roadmap

* Tech features
    * high display resolution
    * better FPS
    * platform portability
* SP campaign
* SP skirmish

Low priority:

* Multiplayer
* Match recording/replay
* UI feature-completeness

## License

The source code is subject to GPLv2 unless specified otherwise.

## Build requirements

* CMake
* FreeType2 development library
* OpenAL development library
* SDL2 development library
* Vulkan development library
* glslc

Other requirements (GLM, libfmt, tracy) are pulled by CMake, or are
optional (GoogleTest).

Windows/MSYS2 and Linux are generally expected to work.

## CMake build options

* `-DUSE_TESTS=1` also build tests
* `-DUSE_TRACY=1` also include tracy annotations

## Running

After building the project, start `zhen(.exe)`.

**TODO**: There is no lookup or config support right now, the game's path is
hardcoded.
