#ifndef EMP_CONSTRAINT_HPP
#define EMP_CONSTRAINT_HPP
#include "core/entity.hpp"
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "scene/transform.hpp"
#include <set>
#include <vector>
namespace emp {
enum class eConstraintType {
    Undefined,
    PointAnchor, //2bodies
};
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
        }point_anchor;
    };

    void solve(float delta_time);
    static Constraint createPointAnchor(Entity anchor, Entity rigidbody, vec2f pinch_point_rotated = vec2f(0.f, 0.f));
private:
    void m_solvePointAnchor(float delta_time);
};
struct ConstraintSystem : public System<Constraint> {
    void update(float delta_time);
};
};
#endif
