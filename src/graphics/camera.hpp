#ifndef EMP_CAMERA_HPP
#define EMP_CAMERA_HPP

#include "math/math_defs.hpp"
#include "scene/scene_defs.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace emp {

class Camera {
public:
    void setOrthographicProjection(
            float left,
            float right,
            float top,
            float bottom,
            float near,
            float far
    );

    void setPerspectiveProjection(
            float fovy, float aspect, float near, float far
    );

    void setViewDirection(
            glm::vec3 position,
            glm::vec3 direction,
            glm::vec3 up = glm::vec3{0.f, -1.f, 0.f}
    );

    void setViewTarget(
            glm::vec3 position,
            glm::vec3 target,
            glm::vec3 up = glm::vec3{0.f, -1.f, 0.f}
    );

    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    [[nodiscard]] const glm::mat4& getProjection() const {
        return projectionMatrix;
    }

    [[nodiscard]] const glm::mat4& getView() const {
        return viewMatrix;
    }

    [[nodiscard]] const glm::mat4& getInverseView() const {
        return inverseViewMatrix;
    }

    [[nodiscard]] glm::vec3 getPosition() const {
        return glm::vec3(inverseViewMatrix[3]);
    }
#if EMP_SCENE_2D
    void setOrthographicProjection(
            float left, float right, float top, float bottom
    );

    void setView(glm::vec2 position, float rotation);
    vec2f convertWorldToScreenPosition(vec2f position);
    vec2f convertWorldToScreenVector(vec2f offset);
#endif

protected:
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};
    glm::mat4 inverseViewMatrix{1.f};
};
} // namespace emp
#endif
