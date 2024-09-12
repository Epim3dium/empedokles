#include "constraint.hpp"
#include "physics_system.hpp"
namespace emp {
    void Constraint::solve(float delta_time) {
        switch(type) {
            case emp::eConstraintType::Penetration:
                solvePenetrationConstraint(*this, delta_time);
                break;
            case emp::eConstraintType::PointAnchor:
                assert(false);
                break;
            case emp::eConstraintType::Undefined:
                assert(false);
                break;
        }

    }
};
