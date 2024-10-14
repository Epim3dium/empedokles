#ifndef EMP_ANIMATED_SPRITE_HPP
#define EMP_ANIMATED_SPRITE_HPP
#include <string>
#include "core/entity.hpp"
#include "graphics/sprite.hpp"
#include "templates/finite_state_machine.hpp"
namespace emp {
class AnimatedSprite {
    typedef FiniteStateMachine<std::string, Entity> StateMachine_t;

    StateMachine_t m_state_machine;
    std::unordered_map<std::string, Sprite> m_animation_frames;
public:
    class Builder {
        StateMachine_t::Builder FSM_builder;
        std::unordered_map<std::string, Sprite> animation_frames;
    public:
        Builder(std::string entry_point, Sprite default_sprite) : FSM_builder(entry_point) {
            animation_frames[entry_point] = default_sprite;
        }
        void addNode(std::string name, Sprite sprite) {
            FSM_builder.addNode(name);
            animation_frames[name] = sprite;
        }
        void addEdge(std::string from, Sprite from_sprite, std::string to, Sprite to_sprite, auto trigger) {
            FSM_builder.addEdge(from, to, trigger);
        }
        friend AnimatedSprite;
    };

    std::string current_frame() const {
        return m_state_machine.state();
    }
    Sprite& sprite() {
        return m_animation_frames.at(current_frame());
    }
    const Sprite& sprite() const {
        return m_animation_frames.at(current_frame());
    }
    void updateState(Entity e) {
        m_state_machine.eval(e);
    }
    AnimatedSprite() : m_state_machine({"undefined"}) {}
    AnimatedSprite(const Builder& builder) : m_state_machine(builder.FSM_builder), m_animation_frames(builder.animation_frames) {}
};
}; // namespace emp
#endif
