#ifndef EMP_COORDINATOR_HPP
#define EMP_COORDINATOR_HPP
#include "component_manager.hpp"
#include "debug/log.hpp"
#include "entity_manager.hpp"
#include "system_manager.hpp"
namespace emp {
class Coordinator {
public:
    void init() {
        // Create pointers to each manager
        m_component_manager = std::make_unique<ComponentManager>();
        m_entity_manager = std::make_unique<EntityManager>();
        m_system_manager = std::make_unique<SystemManager>();
    }

    // Entity methods
    Entity createEntity() {
        return m_entity_manager->createEntity(); 
    }

    void destroyEntity(Entity entity) {
        m_entity_manager->destroyEntity(entity);
        m_component_manager->EntityDestroyed(entity);
        m_system_manager->EntityDestroyed(entity);
    }

    template <typename T>
    void registerComponent() {
        m_component_manager->registerComponent<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        m_component_manager->addComponent<T>(entity, component);

        auto signature = m_entity_manager->getSignature(entity);
        signature.set(m_component_manager->getComponentType<T>(), true);
        m_entity_manager->setSignature(entity, signature);

        m_system_manager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        m_component_manager->removeComponent<T>(entity);

        auto signature = m_entity_manager->getSignature(entity);
        signature.set(m_component_manager->getComponentType<T>(), false);
        m_entity_manager->setSignature(entity, signature);

        m_system_manager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        return m_component_manager->getComponent<T>(entity);
    }

    template <typename T>
    ComponentType getComponentType() {
        return m_component_manager->getComponentType<T>();
    }
    template <typename T>
    bool hasComponent(Entity entity) const {
        return m_component_manager->hasComponent<T>(entity);
    }

    // System methods
    template <typename SystemType, class ...ComponentTypes>
    std::shared_ptr<SystemType> registerSystem() {
        auto system = m_system_manager->registerSystem<SystemType>();

        Signature system_signature;
        (system_signature.set(m_component_manager->getComponentType<ComponentTypes>()), ...);
        m_setSystemSignature<SystemType>(system_signature);
        return system;
    }

private:
    template <typename T>
    void m_setSystemSignature(Signature signature) {
        m_system_manager->setSignature<T>(signature);
    }

    std::unique_ptr<ComponentManager> m_component_manager;
    std::unique_ptr<EntityManager> m_entity_manager;
    std::unique_ptr<SystemManager> m_system_manager;
};
extern Coordinator coordinator;
}; // namespace emp
#endif
