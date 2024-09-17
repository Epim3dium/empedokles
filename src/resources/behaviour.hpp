#ifndef EMP_BEHAVIOUR_HPP
#define EMP_BEHAVIOUR_HPP
#include "core/entity.hpp"
#include "core/system.hpp"
#include <functional>

namespace emp {
    struct Behaviour {
        bool isActive = true;
        std::function<void(Entity)> on_update;
    };
    class BehaviourSystem : public System<Behaviour> {
    public:
        void update() {
            for(auto& e : entities) {
                auto& component = getComponent<Behaviour>(e);
                if(component.isActive) {
                    component.on_update(e);
                }
            }
        }
    };
};
#endif