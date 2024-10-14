#ifndef EMP_COMPONENT_MANAGER_HPP
#define EMP_COMPONENT_MANAGER_HPP
#include <typeinfo>
#include <unordered_map>
#include "core/component.hpp"
#include "core/component_array.hpp"
#include "debug/log.hpp"
namespace emp {
class ComponentManager {
public:
    template <typename T>
    void registerComponent() {
        const char* type_name = typeid(T).name();
        assert(m_component_types.find(type_name) == m_component_types.end() &&
               "Registering component type more than once.");

        m_component_types.insert({type_name, m_next_component_type});
        m_component_arrays.insert(
                {type_name, std::make_shared<ComponentArray<T>>()});
        ++m_next_component_type;
    }

    template <typename T>
    ComponentType getComponentType() const {
        const char* type_name = typeid(T).name();
        assert(m_component_types.find(type_name) != m_component_types.end() &&
               "Component not registered before use.");

        return m_component_types.at(type_name);
    }
    template <typename T>
    bool hasComponent(Entity entity) {
        return getComponentArray<T>().hasData(entity);
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>().InsertData(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>().RemoveData(entity);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        // Get a reference to a component from the array for an entity
        return getComponentArray<T>().GetData(entity);
    }
    template <typename T>
    const T& getComponent(Entity entity) const {
        return getComponentArray<T>().GetData(entity);
    }

    void EntityDestroyed(Entity entity) {
        for (auto const& pair : m_component_arrays) {
            auto const& component = pair.second;

            component->EntityDestroyed(entity);
        }
    }

private:
    std::unordered_map<std::string, ComponentType> m_component_types{};
    ComponentType m_next_component_type{};
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>>
            m_component_arrays{};

    template <typename T>
    ComponentArray<T>& getComponentArray() {
        const char* type_name = typeid(T).name();
        assert(m_component_types.find(type_name) != m_component_types.end() &&
               "Component not registered before use.");

        return *std::static_pointer_cast<ComponentArray<T>>(
                m_component_arrays.at(type_name));
    }
};
}; // namespace emp
#endif
