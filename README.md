# ZHenerals

A just-for-fun journey on reimplementing _Command & Conquer: Generals (Zero Hour)_.

![ZHenerals](https://raw.github.com/MarcelHB/zhenerals/master/title.webp)

## Roadmap

* Tech features
    * high display resolution
    * better FPS
    * platform portability
* SP campaign
* SP skirmish
* better SP AI

Low priority:

* Multiplayer
* Match recording/replay
* UI feature-completeness

## License

The source code is subject to GPLv2 unless specified otherwise. Please consult the `LICENSE` file.

## Build requirements

* CMake
* FreeType2
* OpenAL
* SDL3
* Vulkan
* libAV: libavcodec, libavformat, libavutil, libswresample
* glslc

Other requirements (GLM, libfmt, tracy) are pulled by CMake, or are
optional (GoogleTest).

Windows/MSYS2 and Linux are generally expected to work.

## CMake build options

* `-DUSE_TESTS=1`: also build tests
* `-DUSE_GAME_TESTS=1`: also build game tests (requires game assets to be available)
* `-DUSE_TRACY=1`: also include tracy annotations

## Running

After building the project, start `zhen(.exe)`.

**NOTE**: Modify `game/Config.h` to adjust the path to your English ZH directory.

## Tools

* `BIG`: inspecting and extracting items from `.big` files.
* `decompress`: decompressing asset files
* `w3dview`: stand-alone rendering of model files
