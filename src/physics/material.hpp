#ifndef EMP_MATERIAL_HPP
#define EMP_MATERIAL_HPP
#include "core/component_system.hpp"
namespace emp {
class Material;
typedef ComponentSystem<Material> MaterialSystem;
typedef ComponentInstance<Material, MaterialSystem> MaterialInstance;
struct Material {
    float static_friction = 0.4;
    float dynamic_friction = 0.2;
    float restitution = 0.1;
    float reserved;
    Material(const Material&) = delete;
    Material(Material&&) = delete;
    Material& operator=(const Material&) = delete;
private:
    friend MaterialSystem;
    Material() {}
};
};
#endif
