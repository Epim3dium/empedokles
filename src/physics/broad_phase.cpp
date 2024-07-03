#include "broad_phase.hpp"
#include "math/shapes/AABB.hpp"
#include "physics/physics_system.hpp"
namespace emp {
template<class IterBegin, class IterEnd>
std::vector<CollidingPair>
    SweepBroadPhase::findPotentialPairs(IterBegin begin, IterEnd end) 
{
    std::vector<CollidingPair> result;
    struct SwipeeInformation {
        float x_value;
        //index of owner
        size_t idx;
        //index of convex
        size_t sub_idx;
        AABB aabb;
    };
    std::vector<SwipeeInformation> sweep_along_x;
    size_t index = 0;
    for(auto itr = begin; itr != end; itr++) {
        auto& obj = *itr;
        const auto& shape = obj.collider->constituentConvex(); 
        size_t sub_index = 0;
        for(const auto& convex : shape) {
            AABB aabb = AABB::Expandable();
            for(auto& point : convex) {
                aabb.expandToContain(point);
            }
            sweep_along_x.push_back({aabb.min.x, index, sub_index, aabb});
            sweep_along_x.push_back({aabb.max.x, index, sub_index, aabb});
            sub_index++;
        }
        index++;
    }
    std::sort(sweep_along_x.begin(), sweep_along_x.end(),
        [](const SwipeeInformation& p1, const SwipeeInformation& p2) {
            return p1.x_value < p2.x_value;
        });
    struct OpenedSwipee {
        size_t idx;
        size_t sub_idx;
        AABB aabb;
    };
    std::vector<OpenedSwipee> open;
    for(auto evaluated : sweep_along_x) {
        auto itr = std::find_if(open.begin(), open.end(), 
            [&](const OpenedSwipee& p) {
                return p.idx == evaluated.idx && p.sub_idx == evaluated.sub_idx;
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
            if(other.idx == evaluated.idx) {
                if(other.sub_idx == evaluated.sub_idx) {
                    assert(other.idx != evaluated.idx && "opened convex shape should have already been evaluated");
                }
                continue;
            }
            if(isOverlappingAABBAABB(other.aabb, aabb)) {
                result.push_back({evaluated.idx, other.idx, evaluated.sub_idx, other.sub_idx});
            }
        }
        open.push_back({evaluated.idx, evaluated.sub_idx, aabb});
    }
    return result;
}
};
