#include "animated_sprite.hpp"
namespace emp {
    void AnimatedSprite::m_processSpriteChange(std::string new_sprite_id) {
        auto& moving_sprite = m_moving_sprites.at(new_sprite_id);
        auto& sprite = moving_sprite.sprite;

        m_current_anim_frame_idx = 0;
        sprite.frame = moving_sprite.frames[m_current_anim_frame_idx].frame;
        m_current_frame_lasted_sec = 0.f;
    }
    void AnimatedSprite::m_checkFrameSwitching(float delta_time) {
        m_current_frame_lasted_sec += delta_time;
        auto& moving_sprite = m_moving_sprites.at(current_sprite_frame());
        auto& sprite = moving_sprite.sprite;
        auto& frames = moving_sprite.frames;
        auto& current_frame = frames[m_current_anim_frame_idx];
        if(m_current_frame_lasted_sec < current_frame.duration) {
            return;
        }
        m_current_frame_lasted_sec = 0.f;
        m_current_anim_frame_idx++;

        int max_frame_idx = moving_sprite.frames.size();
        if(moving_sprite.isLooping) {
            m_current_anim_frame_idx %= max_frame_idx;
        }
        m_current_anim_frame_idx = std::min(m_current_anim_frame_idx, max_frame_idx);
        sprite.frame = frames[m_current_anim_frame_idx].frame; 
    }
    void AnimatedSprite::updateState(Entity entity, float delta_time) {
        auto sprite_id_before = current_sprite_frame();
        m_state_machine.eval(entity);
        auto sprite_id_after = current_sprite_frame();
        if(sprite_id_before != sprite_id_after) {
            m_processSpriteChange(sprite_id_after);
            return;
        }
        m_checkFrameSwitching(delta_time);
    }
};
