#ifndef EMP_MATERIAL_HPP
#define EMP_MATERIAL_HPP
namespace emp {
class Material;
struct Material {
    float static_friction = 0.4;
    float dynamic_friction = 0.2;
    float restitution = 0.1;
    float reserved;
    Material() {
    }
};
}; // namespace emp
#endif
