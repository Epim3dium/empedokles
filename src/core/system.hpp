#ifndef EMP_SYSTEM_HPP
#define EMP_SYSTEM_HPP

#include "core/entity.hpp"
#include <set>
namespace emp {
    class System {
    public:
        std::set<Entity> entities;
        virtual void onEntityRemoved(Entity entity) {}
        virtual void onEntityAdded(Entity entity) {}

        System(const System &) = delete;
        System &operator=(const System &) = delete;
        System(System &&) = delete;
        System &operator=(System &&) = delete;
        System(){}
    };

    template<class ...ComponentTypes>
    class SystemOf : public System {};
};

#endif
