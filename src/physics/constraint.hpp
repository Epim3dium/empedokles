#ifndef EMP_CONSTRAINT_HPP
#define EMP_CONSTRAINT_HPP
#include <set>
#include <vector>
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "physics/rigidbody.hpp"
#include "scene/transform.hpp"
namespace emp {
enum class eConstraintType {
    Undefined,
    PointAnchor, // 2bodies
};
struct PositionalCorrectionInfo {
    Entity entity1;
    vec2f center_to_collision1;
    bool isStatic1;
    float inertia1;
    float mass1;
    float generalized_inverse_mass1;
    Entity entity2;
    vec2f center_to_collision2;
    bool isStatic2;
    float inertia2;
    float mass2;
    float generalized_inverse_mass2;
    PositionalCorrectionInfo() {
    }
    PositionalCorrectionInfo(
            vec2f normal,
            Entity e1,
            vec2f r1,
            const Rigidbody* rb1,
            Entity e2,
            vec2f r2,
            const Rigidbody* rb2 = nullptr
    );
};
struct PositionalCorrResult {
    vec2f pos1_correction = vec2f(0, 0);
    float rot1_correction = 0.f;
    vec2f pos2_correction = vec2f(0, 0);
    float rot2_correction = 0.f;

    float delta_lagrange = 0.f;
};
PositionalCorrResult calcPositionalCorrection(
        PositionalCorrectionInfo info,
        float c,
        vec2f normal,
        float delT,
        float compliance = 0.f
);
struct Constraint {
    std::vector<Entity> entity_list;
    float compliance = 0;
    float damping = 1.f;
    bool disabled_collision_between_bodies = false;
    eConstraintType type;

    union {
        struct {
            vec2f relative_position;
            vec2f pinch_point_model;
        } point_anchor;
    };

    void solve(float delta_time, Coordinator& ECS);
    static Constraint createPointAnchor(
            Entity anchor, const Transform* anchor_trans,
            Entity rigidbody,const Transform* rigid_trans,
            vec2f pinch_point_rotated = vec2f(0.f, 0.f)
    );

private:
    void m_solvePointAnchor(float delta_time, Coordinator& ECS);
};
struct ConstraintSystem : public System<Constraint> {
    void update(float delta_time);
};
}; // namespace emp
#endif

