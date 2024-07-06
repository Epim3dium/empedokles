#ifndef EMP_BROADPHASE_HPP
#define EMP_BROADPHASE_HPP
#include <set>
#include <vector>
#include "core/entity.hpp"
namespace emp {
struct CollidingPair {
    Entity e1;
    Entity e2;
    size_t shape_index1;
    size_t shape_index2;
};
class SweepBroadPhase{
public:
    std::vector<CollidingPair> findPotentialPairs(std::set<Entity>::iterator begin, std::set<Entity>::iterator end);
};
};
#endif
