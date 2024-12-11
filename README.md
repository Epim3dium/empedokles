# Empedokles
My dream project of an engine able to run interactive live simulations of fluid, solid and liquid. 
My goal is not to achieve accuracy but to make it playable in real time.
### Requirements
* Basic animation system either bone or sprite based
* Interactive simulation of:
    * Liquid 
    * Solid
    * Fluid
    * Rigidbody
* Interaction between simulation systems
* Cross platform
* min req: M1 mac (without GPU it should work on anything else)
### Specification
* simulations working at runtime (60FPS) (for the cost of precision which I am able to sacrifice)
* controls using keyboard and mouse
### Design
* Pixelart
* Can be very simplistic
* Can be single screen, according to the performance might upscale it to multiscreen in the future
* Single-player (for now)
### Implementation
* C++ as I need very good performance for all the simulation systems
* git for version control
* Main game logic uses hand crafted Entity Component System for speed and flexibility.
* Rendering done from the ground up using vulkan. (Huge thanks to awesome guides: [vulkan-tutorial](https://vulkan-tutorial.com/) and [vkguide](https://vkguide.dev/))
* dependancies:
    * [GLM](https://github.com/g-truc/glm) as a math library, because implementation of all the math functions would be very time consuming.
    * [Vulkan](https://www.vulkan.org/) for graphics for high level access to GPU and corss-platform compatibility
    * [stb image](https://github.com/nothings/stb) to read different types of graphic files and [tiny obj loader](https://github.com/tinyobjloader/tinyobjloader) for .obj files - not fun to code up myself, not my aim with this project
    * [ImGui](https://github.com/ocornut/imgui) for GUIs, once again - not my aim to make a great GUI system, it's more of an addon
    * [gtest](https://github.com/google/googletest) for unit tests

### Evolution:
* I am now aware that running evey simulation on a single thread, on the CPU might be unfeasable, started working on compute shaders for fluid simulation.
* Added unit tests, feel like it's useful to be sure the backbone structure works before any manual testing
