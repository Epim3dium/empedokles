#ifndef EMP_ANIMATED_SPRITE_HPP
#define EMP_ANIMATED_SPRITE_HPP
#include <string>
#include "core/entity.hpp"
#include "debug/log.hpp"
#include "graphics/sprite.hpp"
#include "templates/finite_state_machine.hpp"
namespace emp {
struct MovingSprite {
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
    static MovingSprite singleFrame(Sprite sprite) {
        MovingSprite result;
        result.sprite = sprite;
        result.add(0, INFINITY);
        return result;
    }
};
class AnimatedSprite {
    typedef FiniteStateMachine<std::string, Entity> StateMachine_t;

    StateMachine_t m_state_machine;
    std::unordered_map<std::string, MovingSprite> m_moving_sprites;
    int m_current_anim_frame_idx = 0;
    float m_current_frame_lasted_sec = 0.f;
    void m_processSpriteChange(std::string new_sprite_id);
    void m_checkFrameSwitching(float delta_time);
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
        auto& moving_sprite = m_moving_sprites.at(current_sprite_frame());
        return moving_sprite.sprite;
    }
    const Sprite& sprite() const {
        auto& moving_sprite = m_moving_sprites.at(current_sprite_frame());
        return moving_sprite.sprite;
    }
    void updateState(Entity entity, float delta_time);

    class Builder {
        StateMachine_t::Builder FSM_builder;
        std::unordered_map<std::string, MovingSprite> moving_sprites;
    public:
        Builder(std::string entry_point, const MovingSprite& default_sprite) : FSM_builder(entry_point) {
            moving_sprites[entry_point] = default_sprite;
        }
        void addNode(std::string name, const MovingSprite& sprite) {
            FSM_builder.addNode(name);
            moving_sprites[name] = sprite;
        }
        void addEdge(std::string from, std::string to, auto trigger) {
            FSM_builder.addEdge(from, to, trigger);
        }
        friend AnimatedSprite;
    };
    AnimatedSprite() : m_state_machine({"undefined"}) {}
    AnimatedSprite(const Builder& builder) : m_state_machine(builder.FSM_builder), m_moving_sprites(builder.moving_sprites) {}
};
}; // namespace emp
#endif
