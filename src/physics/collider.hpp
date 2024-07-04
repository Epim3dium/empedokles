#ifndef EMP_COLLIDER_HPP
#define EMP_COLLIDER_HPP
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "scene/transform.hpp"
#include <vector>
namespace emp {
class Collider;


struct Collider {
    typedef std::vector<vec2f> ConvexVertexCloud;
    //potentially concave
    std::vector<vec2f> model_outline;
    std::vector<ConvexVertexCloud> model_shape;

    std::vector<vec2f> transformed_outline;
    std::vector<ConvexVertexCloud> transformed_shape;

    float area;
    float inertia_dev_mass;
    Collider() {}
    Collider(std::vector<vec2f> shape, bool correctCOM = false);
};
//system for updating transfomred collider shapes
class ColliderSystem : public SystemOf<Transform, Collider> {
public:
    void update();
    void onEntityAdded(Entity entity) override;
};
};
#endif
