#include "keyboard_movement_controller.hpp"
#include "core/coordinator.hpp"

// std
#include <limits>

namespace emp {

    void KeyboardMovementController::update(GLFWwindow* window) {
        ;
        for (int key = 32; key < 348; key++) {
            int state = glfwGetKey(window, key);
            GLFW_KEY_K;
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
    }
    void KeyboardMovementController::moveInPlaneXZ(float dt, Entity entity) const {
        if(!coordinator.hasComponent<Transform2D>(entity)) {
            EMP_LOG(WARNING) << "tried to attach movement controller to transformless entity";
            return;
        }

        glm::vec3 rotate{0};
        if (keys.at(mapping.lookRight).held) rotate.y += 1.f;
        if (keys.at(mapping.lookLeft).held)  rotate.y -= 1.f;
        if (keys.at(mapping.lookUp).held)    rotate.x += 1.f;
        if (keys.at(mapping.lookDown).held)  rotate.x -= 1.f;

        // if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        //     entity.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        // }
        // entity.transform.rotation.x = glm::clamp(entity.transform.rotation.x, -1.5f, 1.5f);
        // entity.transform.rotation.y = glm::mod(entity.transform.rotation.y, glm::two_pi<float>());

        float yaw = 0.f /* entity.transform.rotation.y */;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (keys.at(mapping.moveForward).held)  moveDir += forwardDir;
        if (keys.at(mapping.moveBackward).held) moveDir -= forwardDir;
        if (keys.at(mapping.moveRight).held)    moveDir += rightDir;
        if (keys.at(mapping.moveLeft).held)     moveDir -= rightDir;
        if (keys.at(mapping.moveUp).held)       moveDir += upDir;
        if (keys.at(mapping.moveDown).held)     moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            auto& transform = coordinator.getComponent<Transform2D>(entity);
            auto delta = moveSpeed * dt * glm::normalize(moveDir);
            transform.position += vec2f{delta.x, delta.y};
        }
    }
}  // namespace emp
