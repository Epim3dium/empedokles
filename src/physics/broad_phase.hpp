#ifndef EMP_BROADPHASE_HPP
#define EMP_BROADPHASE_HPP
#include "core/entity.hpp"
#include <vector>
namespace emp {
struct CollidingPair {
    Entity e1;
    Entity e2;
    size_t shape_index1;
    size_t shape_index2;
};
class SweepBroadPhase{
public:
    template<class IterBegin, class IterEnd>
    std::vector<CollidingPair> findPotentialPairs(IterBegin begin, IterEnd end);
};
};
#endif
