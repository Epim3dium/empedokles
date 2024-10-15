#include "coordinator.hpp"
namespace emp {
    void Coordinator::init() {
        // Create pointers to each manager
        m_component_manager = std::make_unique<ComponentManager>();
        m_entity_manager = std::make_unique<EntityManager>();
        m_system_manager = std::make_unique<SystemManager>();
    }

    // Entity methods
    Entity Coordinator::createEntity() {
        return m_entity_manager->createEntity(); 
    }
    bool Coordinator::isEntityAlive(Entity entity) const {
        return m_entity_manager->isEntityAlive(entity); 
    }

    void Coordinator::destroyEntity(Entity entity) {
        m_entity_manager->destroyEntity(entity);
        m_component_manager->EntityDestroyed(entity);
        m_system_manager->EntityDestroyed(entity);
    }
    void Coordinator::destroy() {
        delete m_component_manager.release();
        delete m_entity_manager.release();
        delete m_system_manager.release();
    }
    Coordinator coordinator;
};

