#ifndef EMP_ANIMATED_SPRITE_HPP
#define EMP_ANIMATED_SPRITE_HPP
#include <string>
#include "core/entity.hpp"
#include "debug/log.hpp"
#include "graphics/sprite.hpp"
#include "templates/finite_state_machine.hpp"
namespace emp {
struct SpriteFrames {
    struct FrameDuration {
        int frame;
        float duration;
    };
    Sprite sprite;
    bool isLooping = true;
    std::vector<FrameDuration> frames;
    inline void add(int frame, float duration) {
        frames.push_back({frame, duration});
    }
    static SpriteFrames singleFrame(Sprite sprite) {
        SpriteFrames result;
        result.sprite = sprite;
        result.add(0, INFINITY);
        return result;
    }
};
class AnimatedSprite {
    typedef FiniteStateMachine<std::string, Entity> StateMachine_t;

    StateMachine_t m_state_machine;
    std::unordered_map<std::string, SpriteFrames> m_animation_frames;
    int animation_frame = 0;
public:
    vec2f position_offset = vec2f(0, 0);
    bool flipX = false;
    bool flipY = false;
    // used for shaders stuff
    glm::vec4 color;

    std::string current_sprite_frame() const {
        return m_state_machine.state();
    }
    Sprite& sprite() {
        auto& all_frames = m_animation_frames.at(current_sprite_frame());
        // auto frame_index = all_frames.frames[animation_frame].frame;
        // all_frames.sprite.frame = frame_index;
        return all_frames.sprite;
    }
    const Sprite& sprite() const {
        auto& all_frames = m_animation_frames.at(current_sprite_frame());
        return all_frames.sprite;
    }
    void updateState(Entity e) {
        m_state_machine.eval(e);
    }

    class Builder {
        StateMachine_t::Builder FSM_builder;
        std::unordered_map<std::string, SpriteFrames> animation_frames;
    public:
        Builder(std::string entry_point, const SpriteFrames& default_sprite) : FSM_builder(entry_point) {
            animation_frames[entry_point] = default_sprite;
        }
        void addNode(std::string name, const SpriteFrames& sprite) {
            FSM_builder.addNode(name);
            animation_frames[name] = sprite;
        }
        void addEdge(std::string from, std::string to, auto trigger) {
            FSM_builder.addEdge(from, to, trigger);
        }
        friend AnimatedSprite;
    };
    AnimatedSprite() : m_state_machine({"undefined"}) {}
    AnimatedSprite(const Builder& builder) : m_state_machine(builder.FSM_builder), m_animation_frames(builder.animation_frames) {}
};
}; // namespace emp
#endif
