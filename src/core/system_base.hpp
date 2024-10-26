#ifndef EMP_SYSTEM_BASE_HPP
#define EMP_SYSTEM_BASE_HPP

#include <set>
#include "core/component.hpp"
#include "core/entity.hpp"
namespace emp {
struct SystemManager;
class SystemBase {
public:
    const std::set<Entity>& getEntities() {
        return entities;
    }
    virtual void onEntityRemoved(Entity entity) {
    }
    virtual void onEntityAdded(Entity entity) {
    }

    SystemBase(const SystemBase&) = delete;
    SystemBase& operator=(const SystemBase&) = delete;
    SystemBase(SystemBase&&) = delete;
    SystemBase& operator=(SystemBase&&) = delete;
    virtual ~SystemBase() {
    }

    friend SystemManager;
protected:
    std::set<Entity> entities;
    SystemBase() {
    }
};

template <class... ComponentTypes>
class SystemOf : public SystemBase {
protected:
    SystemOf() {
    }
};
}; // namespace emp

#endif
