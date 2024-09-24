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
### Specification
* simulations working at runtime (60FPS) (for the cost of precision which I am able to sacrifice)
* min req: M1 mac (without GPU it should work on anything else)
### Design
* Pixelart
* Can be very simplistic
* Can be single screen, according to the perforcmance might upscale it to multiscreen in the future
### Implementation
* C++ as I need very good performance for all the simulation systems
    * GLM as a math library, because implementation of all the math functions would be very time consuming.
* Vulkan for graphics for high level access to GPU and corss-platform compatibility
* git for version control

### Evolution:
