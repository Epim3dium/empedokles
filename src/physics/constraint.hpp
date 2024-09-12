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

    void solve(float delta_time);
};
};
#endif
