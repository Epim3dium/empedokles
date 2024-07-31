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
            int move_left = GLFW_KEY_A;
            int move_right = GLFW_KEY_D;
            int move_forward = GLFW_KEY_W;
            int move_backward = GLFW_KEY_S;
            int move_up = GLFW_KEY_E;
            int move_down = GLFW_KEY_Q;
            int look_left = GLFW_KEY_LEFT;
            int look_right = GLFW_KEY_RIGHT;
            int look_up = GLFW_KEY_UP;
            int look_down = GLFW_KEY_DOWN;
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
