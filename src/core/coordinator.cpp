#include "coordinator.hpp"
#include "debug/debug.hpp"
namespace emp {
const  Entity Coordinator::world() {
    return 0U;
}
void Coordinator::init() {
    // Create pointers to each manager
    m_component_manager = std::make_unique<ComponentManager>();
    m_entity_manager = std::make_unique<EntityManager>();
    m_system_manager = std::make_unique<SystemManager>();
    auto world_entity = createEntity();
    assert(world_entity == world());
}

// Entity methods
Entity Coordinator::createEntity() {
    auto result = m_entity_manager->createEntity();
    EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "entity created: " << result;)
    return result;
}
bool Coordinator::isEntityAlive(Entity entity) const {
    return m_entity_manager->isEntityAlive(entity);
}

void Coordinator::destroyEntity(Entity entity) {
    EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "entity destroyed: " << entity;)
    m_entity_manager->destroyEntity(entity);
    m_component_manager->EntityDestroyed(entity);
    m_system_manager->EntityDestroyed(entity);
}
void Coordinator::destroy() {
    EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "coordinator destroyed";)
    delete m_component_manager.release();
    delete m_entity_manager.release();
    delete m_system_manager.release();
}
};

