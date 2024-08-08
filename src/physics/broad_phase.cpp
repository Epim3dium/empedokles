#include "broad_phase.hpp"
#include "math/shapes/AABB.hpp"
#include "physics/physics_system.hpp"
#include "core/coordinator.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
#include "physics/collider.hpp"

namespace emp {
    std::vector<CollidingPair> SweepBroadPhase::findPotentialPairs(std::set<Entity>::iterator begin, std::set<Entity>::iterator end) {
    std::vector<CollidingPair> result;
    struct SwipeeInformation {
        float x_value;
        //index of owner
        Entity entity;
        //index of convex
        size_t sub_idx;
        AABB aabb;
    };
    std::vector<SwipeeInformation> sweep_along_x;
    size_t index = 0;
    for(auto itr = begin; itr != end; itr++) {
        const Entity entity = *itr;

        const auto shape_ptr = coordinator.getComponent<Collider>(entity);
        assert(shape_ptr != nullptr);
        const auto& shape = shape_ptr->transformed_shape;

        size_t sub_index = 0;
        for(const auto& convex : shape) {
            AABB aabb = AABB::Expandable();
            for(auto& point : convex) {
                aabb.expandToContain(point);
            }
            sweep_along_x.push_back({aabb.min.x, entity, sub_index, aabb});
            sweep_along_x.push_back({aabb.max.x, entity, sub_index, aabb});
            sub_index++;
        }
        index++;
    }
    std::sort(sweep_along_x.begin(), sweep_along_x.end(),
        [](const SwipeeInformation& p1, const SwipeeInformation& p2) {
            return p1.x_value < p2.x_value;
        });
    struct OpenedSwipee {
        Entity entity;
        size_t sub_idx;
        AABB aabb;
    };
    std::vector<OpenedSwipee> open;
    for(auto evaluated : sweep_along_x) {
        auto itr = std::find_if(open.begin(), open.end(), 
            [&](const OpenedSwipee& p) {
                return p.entity == evaluated.entity && p.sub_idx == evaluated.sub_idx;
            });
        //delete if already was opened
        if(itr != open.end()) {
            std::swap(*itr, open.back());
            open.pop_back();
            continue;
        }
        
        //compare against all other opened 
        auto aabb = evaluated.aabb;
        for(auto other : open) {
            if(other.entity == evaluated.entity) {
                if(other.sub_idx == evaluated.sub_idx) {
                    assert(other.entity != evaluated.entity && "opened convex shape should have already been evaluated");
                }
                continue;
            }
            if(isOverlappingAABBAABB(other.aabb, aabb)) {
                result.push_back({evaluated.entity, other.entity, evaluated.sub_idx, other.sub_idx});
            }
        }
        open.push_back({evaluated.entity, evaluated.sub_idx, aabb});
    }
    return result;
}
};
