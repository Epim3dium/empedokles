#ifndef EMP_COMPONENT_MANAGER_HPP
#define EMP_COMPONENT_MANAGER_HPP
#include "core/component.hpp"
#include "core/component_array.hpp"
#include <typeinfo>
#include <unordered_map>
namespace emp {
    class ComponentManager
{
public:
    template <typename T>
    void registerComponent() {
        const char* typeName = typeid(T).name();
        assert(m_component_types.find(typeName) == m_component_types.end() && "Registering component type more than once.");

        m_component_types.insert({typeName, m_next_component_type});
        m_component_arrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
        ++m_next_component_type;
    }

    template <typename T>
    ComponentType getComponentType() const {
        const char* typeName = typeid(T).name();
        assert(m_component_types.find(typeName) != m_component_types.end() && "Component not registered before use.");

        return m_component_types.at(typeName);
    }
    template <typename T>
    bool hasComponent(Entity entity) const {
        return GetComponentArray<T>()->hasData(entity);
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        GetComponentArray<T>()->RemoveData(entity);
    }

    template <typename T>
    T& getComponent(Entity entity) {
        // Get a reference to a component from the array for an entity
        return GetComponentArray<T>()->GetData(entity);
    }
    template <typename T>
    const T& getComponent(Entity entity) const {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(Entity entity) {
        for (auto const& pair : m_component_arrays) {
            auto const& component = pair.second;

            component->EntityDestroyed(entity);
        }
    }

private:
	std::unordered_map<const char*, ComponentType> m_component_types{};
    ComponentType m_next_component_type{};
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_component_arrays{};

	template<typename T>
	const std::shared_ptr<ComponentArray<T>> GetComponentArray() const
	{
        return GetComponentArray<T>();
	}
	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray()
	{
		const char* typeName = typeid(T).name();
		assert(m_component_types.find(typeName) != m_component_types.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[typeName]);
	}
};
};
#endif
