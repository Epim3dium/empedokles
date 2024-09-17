#include "sprite.hpp"
namespace emp {
    std::unordered_map<std::string, std::unique_ptr<Sprite>> SpriteRenderer::m_sprite_table;
    Sprite::Sprite(Texture tex, AABB tex_rect, vec2f size) : m_texture(tex), m_rect(tex_rect) {
        auto tex_size = tex.texture().getSize();
        if(tex_size.x > tex_size.y) {
            m_size.x = 1.f;
            m_size.y = tex_size.y / tex_size.x;
        }else {
            m_size.y = 1.f;
            m_size.x = tex_size.x / tex_size.y;
        }
    }
    Sprite& SpriteRenderer::sprite() {
        assert(isLoaded(m_id) && "texture must be created before use");
        return *m_sprite_table.at(m_id);
    }
    const Sprite& SpriteRenderer::sprite() const {
        assert(isLoaded(m_id) && "texture must be created before use");
        return *m_sprite_table.at(m_id);
    }
    void SpriteRenderer::create(std::string id, Texture tex, AABB tex_rect, vec2f size) {
        auto spr = std::make_unique<Sprite>(tex, tex_rect, size);
        assert(!isLoaded(id) && "trying to override existing texture id");
        m_sprite_table[id] = std::move(spr);
    }
};
