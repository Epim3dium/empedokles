#include "broad_phase.hpp"
#include "math/shapes/AABB.hpp"
#include "physics/physics_system.hpp"
#include "core/coordinator.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
#include "physics/collider.hpp"

namespace emp {
    // Entity e1;
    // Entity e2;
    // size_t shape_index1;
    // size_t shape_index2;
    std::vector<CollidingPair> SweepBroadPhase::findPotentialPairs(std::set<Entity>::iterator begin, std::set<Entity>::iterator end) {
    std::vector<CollidingPair> result;
    struct Object {
        Entity owner;
        AABB aabb;
        size_t sub_shape_idx;
    };
    std::vector<Object> objects_sorted;
    for(auto itr = begin; itr != end; itr++) {
        auto& col = *coordinator.getComponent<Collider>(*itr);
        size_t i = 0;
        for(const auto& shape : col.transformed_shape) {
            objects_sorted.push_back({*itr, AABB::CreateFromVerticies(shape), i});
            i++;
        }
    }
	std::sort(objects_sorted.begin(), objects_sorted.end(),
		[](const Object& a, const Object& b) -> bool
	{ return a.aabb.min.x < b.aabb.min.x; });

    std::vector<int> opened; 
    for (auto i = 0; i < objects_sorted.size(); ++i) {
        for (auto j = 0; j < opened.size(); ++j) {
            if(objects_sorted[i].aabb.min.x > objects_sorted[opened[j]].aabb.max.x) {
                opened.erase(opened.begin() + j);
                j--;
            }else if(objects_sorted[i].owner != objects_sorted[opened[j]].owner) {
                if(isOverlappingAABBAABB(objects_sorted[i].aabb, objects_sorted[opened[j]].aabb)) {
                    const auto& obj1 = objects_sorted[i];
                    const auto& obj2 = objects_sorted[opened[j]];
                    result.push_back({obj1.owner, obj2.owner, obj1.sub_shape_idx, obj2.sub_shape_idx}); 
                }
            }
        }
        opened.push_back(i);
    }
	return  result;
}

};
