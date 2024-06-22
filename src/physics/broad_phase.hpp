#ifndef EMP_BROADPHASE_HPP
#define EMP_BROADPHASE_HPP
#include <vector>
namespace emp {
struct PhysicsManagerEntry;
class BroadPhaseSolver {
public:
    struct CollidingPair {
        size_t idx1;
        size_t idx2;
        size_t subShapeIdx1;
        size_t subShapeIdx2;
        CollidingPair(size_t i1, size_t i2, size_t si1, size_t si2) : idx1(i1), idx2(i2), subShapeIdx1(si1), subShapeIdx2(si2) {}
    };
    virtual std::vector<CollidingPair> findPotentialPairs(const std::vector<PhysicsManagerEntry>& objects) = 0;
    virtual ~BroadPhaseSolver() {}
};
class SweepBroadPhase : public BroadPhaseSolver{
public:
    std::vector<CollidingPair> findPotentialPairs(const std::vector<PhysicsManagerEntry>& objects) override final;
};
};
#endif
