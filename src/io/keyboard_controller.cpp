#include "keyboard_controller.hpp"
#include "core/coordinator.hpp"

// std
#include <limits>

namespace emp {

    void KeyboardControllerSystem::update(GLFWwindow* window) {
        for (int key = 32; key < 348; key++) {
            int state = glfwGetKey(window, key);
            if (state == GLFW_PRESS) {
                if (!keys[key].held) {
                    keys[key].pressed = true;
                } else {
                    keys[key].pressed = false;
                }
                keys[key].held = true;
            } else if (state == GLFW_RELEASE) {
                keys[key].held = false;
                keys[key].pressed = false;
                keys[key].released = true;
            } else {
                keys[key].released = false;
                keys[key].pressed = false;
            }
        }
        for(auto e : entities) {
            auto& controller = getComponent<KeyboardController>(e);
            controller.m_key_states.clear();
            for(auto binding : controller.m_mappings) {
                const auto& state = keys[binding.second];

                controller.m_key_states[binding.first] = state;
            }
        }
    }
    vec2f KeyboardController::movementInPlane2D() {
        glm::vec3 moveDir{0.f};
        vec3f upDir{0.f, -1.f, 0.f};
        vec3f rightDir{-1.f, 0.f, 0.f};
        if (m_key_states[eKeyMappings::MoveRight].held)    moveDir += rightDir;
        if (m_key_states[eKeyMappings::MoveLeft].held)     moveDir -= rightDir;
        if (m_key_states[eKeyMappings::MoveUp].held)       moveDir += upDir;
        if (m_key_states[eKeyMappings::MoveDown].held)     moveDir -= upDir;
        if(moveDir == glm::vec3(0.f))
            return moveDir;
        return glm::normalize(moveDir);
    }
}  // namespace emp
