# Dreco Engine - Vulkan
Project for personal fun of writing vulkan renderer and an game engine.

## Targets
* Environment 
  * Std::Cpp17 (with ability to upgrade std::Cpp20+)
  * CMake
  * Crossplatform support, at least: Windows, Linux
* Vulkan
  * No different renderer backends, Vulkan as one and only
  * Design for high realtime perfomance
  * Implement dynamic lights, shadows
  * Implement PBR support
  * Threaded rendering
* Engine 
  * Math library
  * Threaded tasks
  * Assets library
* Editor
  * Implement editor with ImGui
  * Make engine actually usable
* Game
  * Create game that utilizes editor and engine

I'm not expecting myself to handle every target, but its fun have something to work towards.

  
## Build
[![Ubuntu](https://github.com/GloryOfNight/dreco-engine-vulkan/actions/workflows/ubuntu_cmake.yml/badge.svg)](https://github.com/GloryOfNight/dreco-engine-vulkan/actions/workflows/ubuntu_cmake.yml)
[![Windows](https://github.com/GloryOfNight/dreco-engine-vulkan/actions/workflows/windows_cmake.yml/badge.svg)](https://github.com/GloryOfNight/dreco-engine-vulkan/actions/workflows/windows_cmake.yml)

 ## Build Dependencies
- CMake at least 3.19.0 ([link](https://cmake.org/))
- Vulkan SDK ([link](https://vulkan.lunarg.com/))
- SDL2 ([link](https://github.com/libsdl-org/SDL))
