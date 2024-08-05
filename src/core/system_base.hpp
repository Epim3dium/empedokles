#ifndef EMP_SYSTEM_BASE_HPP
#define EMP_SYSTEM_BASE_HPP

#include "core/component.hpp"
#include "core/entity.hpp"
#include <set>
namespace emp {
    class SystemBase {
    public:
        std::set<Entity> entities;
        virtual void onEntityRemoved(Entity entity) {}
        virtual void onEntityAdded(Entity entity) {}

        SystemBase(const SystemBase &) = delete;
        SystemBase &operator=(const SystemBase &) = delete;
        SystemBase(SystemBase &&) = delete;
        SystemBase &operator=(SystemBase &&) = delete;
        virtual ~SystemBase() {}
    protected:
        SystemBase(){}
    };

    template<class ...ComponentTypes>
    class SystemOf : public SystemBase {
    protected:
        SystemOf() {}
    };
};

#endif
