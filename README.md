# Empedokles
My dream project of an engine able to run interactive live simulations of fluid, solid and liquid. 
My goal is not to achieve accuracy but to make it playable in real time.
![basic_scene](https://github.com/Epim3dium/empedokles/blob/39062b4dae5caba00d29362d9f37a9e74a699d27/assets/captures/KnightShowcase.gif)
# ✅ DONE:
- collision
    - added more constraints
        - anchored swivel
        - unanchored swivel
        - anchored fixed 
        - unanchored fixed 
- collision
    - collision optimization
        - collision idle state
    - collision triggers
- graphics
    - gui - mainly for debuggin for now
    - resizable window
    - render system abstraction
    - basic animation system
        - animated sprites
        - finite state machine animated sprite switching
- basic editor
- constraints as components

### Editor:
![editor](https://github.com/Epim3dium/empedokles/blob/a870e205c718a59be28882cd5341082e882f092d/assets/captures/EditorView.png)

### 🎯 Requirements (Target)
* Basic animation system either bone or sprite based
* Interactive simulation of:
    * Liquid 
    * Solid
    * Fluid
    * Rigidbody
* Interaction between simulation systems
* Cross platform
* Single-player (for now)
* min req: M1 mac (without GPU it should work on anything else)
### 📐 Specification
* simulations working at runtime (60FPS) (for the cost of precision which I am able to sacrifice)
* controls using keyboard and mouse
### 🎨 Art & Design
* Pixelart
* Can be very simplistic
### </> Implementation
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
