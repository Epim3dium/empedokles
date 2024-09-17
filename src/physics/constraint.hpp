#ifndef EMP_CONSTRAINT_HPP
#define EMP_CONSTRAINT_HPP
#include "core/entity.hpp"
#include "math/math_defs.hpp"
#include <set>
#include <vector>
namespace emp {
enum class eConstraintType {
    Undefined,
    PointAnchor, //2bodies
};
struct Constraint {
    
    std::vector<Entity> entity_list;
    float stiffness = 1.f;
    bool disabled_collision_between_bodies = false;
    eConstraintType type;
    
    union {
        struct {
            vec2f relative_position;
        }point_anchor;
    };

    void solve(float delta_time);
    static Constraint createPointAnchor(Entity anchor, Entity rigidbody);
private:
    void m_solvePointAnchor(float delta_time);
};
};
#endif
