# JFKMW Public Release Repository
![build](https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/actions/workflows/build-all.yml/badge.svg) See actions https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/actions

This is the new JFKMW Repository with the latest major version (>= 4.0.0), a mario fan-game/engine dedicated to trying to replicate the original Super Mario World as much as possible while introducing new features for limitless modding potential, and a multiplayer client/server system for playing custom content made with the game.

The code hosted here contains no copyrighted assets or any stolen code. Most things have been reverse engineered and recoded completely from the original game, while also trying to keep compatibility with custom resources made for the original game.

This code is all licensed under AGPLv3, commercial usage heavily discouraged (see License), after all it's a homage to one of our favorite games and meant as a free platform for expression and creativity.

# Features
* Engine re-coded in C++, player physics nearly 100% accurate to the original game aside some minor sacrifices like a total collision rewrite, amongst other things.
* Multiplayer client/server architecture with syncing support to play the game together, without having to have the same files as the host of the server.
* Ability to distribute mods, using the modpack feature included in the game (in releases, Packs/ folder.)
* Powerful rendering engine to replicate SNES rendering pipeline, with major improvements and additions, fast 4bpp renderer/rasterizer and OpenGL backend.
* Lua as the main language for programming content, mods, etc. Possible ASM support in the future.
* Full support for custom assets, music, custom sprites (enemies and objects), custom blocks, among other things.

While the game has received a lot of maintenance over the months, it hasn't seen a public release until now, and there's a lot of things to tidy up in the codebase, so that might take a while.

# How can I contribute to the project or help?
If you know how to code, submit fixes. If you like to test stuff, make levels and test the craziest stuff you can think of and try to find bugs, then report them in issues. Everything helps! We especially need help on making the game physics accurate. Also if a multiplayer server is up, help us test the netcode by just playing the game.

# About releases
Releases won't be seen until a proper starter pack can be made for the game with 100% original content. You are supposed to supply the game the assets it requires, more shall be documented later.

# Documentation
Pack format: http://htmlpreview.github.io/?https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/blob/main/documentation/Pack%20Format.html
Lua Documentation: http://htmlpreview.github.io/?https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/blob/main/documentation/Lua%20Documentation.html
Sprite Programming: http://htmlpreview.github.io/?https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/blob/main/documentation/Sprite%20Programming.html
RAM Map: http://htmlpreview.github.io/?https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/blob/main/documentation/RAM%20Map.html
Solar Energy (Level Editor): http://htmlpreview.github.io/?https://github.com/AVeryPainedRaspcallion/RaspcallionJFKMW/blob/main/documentation/Solar%20Energy%20And%20Level%20content%20Creation.html

# Requirements / Dependencies / Compiling
You can use the premake script included to build the game, if you want to build manually, here are the required dependencies:
* SDL2
* OpenGL (2.0 or higher, legacy)
* SDL_image
* SDL_mixer (preferably custom build compiled with fluidsynth support for MIDI playback)
* SFML (for network protocols)
* Moonjit as Lua backend (https://github.com/moonjit/moonjit)
* LZ4 1.9.3 (version used, can be more recent)