#include "keyboard_controller.hpp"
#include "core/coordinator.hpp"
#include "math/math_func.hpp"

// std
#include <limits>

namespace emp {

void KeyboardController::update(
        Window& window, const Transform& camera_transform
) {
    double xpos, ypos;
    glfwGetCursorPos(window.getGLFWwindow(), &xpos, &ypos);
    xpos -= window.getExtent().width / 2.f;
    ypos -= window.getExtent().height / 2.f;
    m_mouse_pos = {xpos, ypos};
    m_global_mouse_pos = transformPoint(camera_transform.global(), m_mouse_pos);

    for (int key = 32; key < 348; key++) {
        int state = glfwGetKey(window.getGLFWwindow(), key);
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
    m_key_states.clear();
    for (auto binding : m_mappings) {
        const auto& state = keys[binding.second];

        m_key_states[binding.first] = state;
    }
}
vec2f KeyboardController::movementInPlane2D() {
    glm::vec3 moveDir{0.f};
    vec3f upDir{0.f, -1.f, 0.f};
    vec3f rightDir{1.f, 0.f, 0.f};
    if (m_key_states[eKeyMappings::MoveRight].held)
        moveDir += rightDir;
    if (m_key_states[eKeyMappings::MoveLeft].held)
        moveDir -= rightDir;
    if (m_key_states[eKeyMappings::MoveUp].held)
        moveDir += upDir;
    if (m_key_states[eKeyMappings::MoveDown].held)
        moveDir -= upDir;
    if (moveDir == glm::vec3(0.f))
        return moveDir;
    return glm::normalize(moveDir);
}
vec2f KeyboardController::lookingInPlane2D() {
    glm::vec3 moveDir{0.f};
    vec3f upDir{0.f, -1.f, 0.f};
    vec3f rightDir{-1.f, 0.f, 0.f};
    if (m_key_states[eKeyMappings::LookRight].held)
        moveDir += rightDir;
    if (m_key_states[eKeyMappings::LookLeft].held)
        moveDir -= rightDir;
    if (m_key_states[eKeyMappings::LookUp].held)
        moveDir += upDir;
    if (m_key_states[eKeyMappings::LookDown].held)
        moveDir -= upDir;
    if (moveDir == glm::vec3(0.f))
        return moveDir;
    return glm::normalize(moveDir);
}
} // namespace emp
