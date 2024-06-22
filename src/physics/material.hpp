#ifndef EMP_MATERIAL_HPP
#define EMP_MATERIAL_HPP
namespace emp {
struct Material {
    float static_friction = 0.8;
    float dynamic_friction = 0.4;
    float restitution = 0.1;
    Material(const Material&) = delete;
    Material(Material&&) = delete;
    Material& operator=(const Material&) = delete;
    Material() {}
};
};
#endif
