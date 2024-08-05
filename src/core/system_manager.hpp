#ifndef EMP_SYSTEM_MANAGER_HPP
#define EMP_SYSTEM_MANAGER_HPP
#include "core/component.hpp"
#include "core/system_base.hpp"
#include "core/entity.hpp"
#include <set>
#include <unordered_map>
namespace emp {
class SystemManager {
public:
    template <typename T, class ...InitializerValues>
    std::shared_ptr<T> registerSystem(InitializerValues... inits) {
        const char* typeName = typeid(T).name();
        assert(m_systems.find(typeName) == m_systems.end() && "Registering system more than once.");

        auto system = std::make_shared<T>(inits...);
        m_systems.insert({typeName, system});
        return system;
    }

    template <typename T>
    void setSignature(Signature signature) {
        const char* typeName = typeid(T).name();
        assert(m_systems.find(typeName) != m_systems.end() && "System used before registered.");

        m_signatures.insert({typeName, signature});
    }

    void EntityDestroyed(Entity entity) {
        for (auto const& pair : m_systems) {
            auto const& system = pair.second;

            system->entities.erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, Signature entitySignature) {
        for (auto const& pair : m_systems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = m_signatures[type];
            const bool contained = system->entities.contains(entity);

            if ((entitySignature & systemSignature) == systemSignature) {
                system->entities.insert(entity);
                if(!contained) {
                    system->onEntityAdded(entity);
                }
            }
            else  {
                system->entities.erase(entity);
                if(contained) {
                    system->onEntityRemoved(entity);
                }
            }
        }
    }

private:
    std::unordered_map<const char*, Signature> m_signatures{};
    std::unordered_map<const char*, std::shared_ptr<SystemBase>> m_systems{};
};
}; // namespace emp
#endif
