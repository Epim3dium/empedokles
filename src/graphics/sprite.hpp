#ifndef EMP_SPRITE_HPP
#define EMP_SPRITE_HPP
#include "math/shapes/AABB.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex.hpp"
namespace emp {
    struct Sprite {
        Texture m_texture;
        AABB m_rect;
        vec2f m_size;

    public:
        AABB rect() const { return m_rect; }
        vec2f size() const { return m_size; }
        TextureAsset& texture() { return m_texture.texture(); }

        vec2f pivot = vec2f(0, 0);
        
        vec2f position_offset = vec2f(0, 0);
        float rotation_offset = 0.f;
        vec2f scale_offset = vec2f(1.f, 1.f);

        //size set by default
        Sprite(Texture tex, AABB tex_rect, vec2f size = vec2f(0.f, 0.f));

        static const Vertex verticies[6];
    };
    class SpriteRenderer {
        std::string m_id;
        static std::unordered_map<std::string, std::unique_ptr<Sprite>> m_sprite_table;
    public:
        bool flipX = false;
        bool flipY = false;
        glm::vec4 color;

        static void destroyAll() { m_sprite_table.clear(); }
        std::string getID() const {return m_id; }
        static bool isLoaded(std::string id) { return m_sprite_table.contains(id); }

        Sprite& sprite();
        const Sprite& sprite() const;
        static void create(std::string id, Texture tex, AABB tex_rect, vec2f size = vec2f(0.f, 0.f));
        SpriteRenderer() : m_id("undefined") {}
        SpriteRenderer(std::string model_id) : m_id(model_id) {
            assert(isLoaded(model_id) && "texture must be first created");
        }
    };
};
#endif
