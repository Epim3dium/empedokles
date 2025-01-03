#include "prefab.hpp"
#include "scene/register_scene_types.hpp"
namespace emp {

void Prefab::write(IBlobWriter& writer) {
}
void Prefab::read(IBlobReader& reader) {
}
Prefab::Prefab(Coordinator& ECS, Entity prefab_source) {
    registerSceneTypes(environment);
    m_mapping[prefab_source] = environment.world();
    m_checkAndAddComponents(ECS, prefab_source);
}
void Prefab::m_checkAndAddComponents(Coordinator& ECS, Entity owner) {
    m_checkAndAddFromPack(ECS, owner, AllComponentTypes());
}

}
