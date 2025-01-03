#ifndef EMP_PREFAB_HPP
#define EMP_PREFAB_HPP
#include <unordered_map>
#include "core/coordinator.hpp"
#include "io/serializer.hpp"
#include "templates/type_pack.hpp"
namespace emp {
class Prefab {
    Coordinator environment;
    std::unordered_map<Entity, Entity> m_mapping;

    template<typename ComponentT>
    void m_checkAndAdd(Coordinator& ECS, Entity owner);
    template<typename ...Types>
    void m_checkAndAddFromPack(Coordinator& ECS, Entity owner, TypePack<Types...>);

    void m_checkAndAddComponents(Coordinator& ECS, Entity owner);
public:
    void write(IBlobWriter& writer);
    void read(IBlobReader& reader);

    Entity clone(Coordinator& ECS);

    //copying from running ECS
    Prefab(Coordinator& ECS, Entity prefab_source);
};
template<typename ComponentT>
void Prefab::m_checkAndAdd(Coordinator& ECS, Entity owner) {
    auto* comp_ptr = ECS.getComponent<ComponentT>(owner);
    if(comp_ptr == nullptr) {
        return;
    }
    auto mapped_owner = m_mapping.at(owner);
    assert(environment.isEntityAlive(mapped_owner) && "tried to match components before mapping entity");
    environment.addComponent(mapped_owner, *comp_ptr);
}
template<typename ...Types>
void Prefab::m_checkAndAddFromPack(Coordinator& ECS, Entity owner, TypePack<Types...>) {
    (m_checkAndAdd<Types>(ECS, owner), ...);
}
};
#endif
