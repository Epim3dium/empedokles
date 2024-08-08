#ifndef EMP_SYSTEM_HPP
#define EMP_SYSTEM_HPP
#include "system_base.hpp"
#include "coordinator.hpp"

namespace emp {
    template<typename ...Components>
    class System : public SystemOf<Components...> {
    public:
        template<class T>
        T& getComponent(Entity entity) {
            static_assert((std::is_same<T,Components>::value || ...), "must get component contained in this system");
            return *coordinator.getComponent<T>(entity);
        }
        template<class T>
        const T& getComponent(Entity entity) const {
            static_assert((std::is_same<T,Components>::value || ...), "must get component contained in this system");
            return *coordinator.getComponent<T>(entity);
        }

    };
};

#endif
