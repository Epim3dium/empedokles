#pragma once

#include "graphics/model_system.hpp"
#include "window.hpp"

namespace emp {
    class KeyboardMovementController {
    public:
        struct KeyState {
            bool held = false;
            bool pressed = false;
            bool released = false;
        };
    private:
        std::unordered_map<int, KeyState> keys;
    public:
        inline const std::unordered_map<int, KeyState>& getKeys() const {
            return keys;
        }
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void update(GLFWwindow* window);
        vec2f movementInPlane2D() const;
        struct XZMovement {
            vec3f pos_diff{};
            vec3f rot_diff{};
        };
        XZMovement movementInPlaneXZ() const;

        KeyMappings mapping{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    };
}  // namespace emp
