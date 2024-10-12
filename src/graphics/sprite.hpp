#ifndef EMP_SPRITE_HPP
#define EMP_SPRITE_HPP
#include "graphics/texture.hpp"
#include "graphics/vertex.hpp"
#include "math/shapes/AABB.hpp"
namespace emp {
struct Sprite {
    Texture m_texture;
    AABB m_rect;
    vec2f m_size;

public:
    AABB rect() const {
        return m_rect;
    }
    vec2f size() const {
        return m_size;
    }
    TextureAsset& texture() {
        return m_texture.texture();
    }

    vec2f pivot = vec2f(0, 0);

    vec2f position_offset = vec2f(0, 0);
    float rotation_offset = 0.f;
    vec2f scale_offset = vec2f(1.f, 1.f);

    // size set by default
    Sprite(Texture tex, AABB tex_rect, vec2f size = vec2f(0.f, 0.f));

    static constexpr uint32_t s_vertex_count = 6U;
    static const Vertex s_verticies[s_vertex_count];
    static std::unique_ptr<Buffer> s_vertex_buffer;

    static void init(Device& device);
    static void bind(VkCommandBuffer commandBuffer);
    static void draw(VkCommandBuffer commandBuffer);

    static Sprite& create(
            std::string id,
            Texture tex,
            AABB tex_rect,
            vec2f size = vec2f(0.f, 0.f)
    );
    static std::unordered_map<std::string, std::unique_ptr<Sprite>>
            s_sprite_table;
    static void destroyAll() {
        s_sprite_table.clear();
    }
    static bool isLoaded(std::string id) {
        return s_sprite_table.contains(id);
    }
};
class SpriteRenderer {
    std::string m_id;

public:
    bool flipX = false;
    bool flipY = false;
    glm::vec4 color;
    std::string getID() const {
        return m_id;
    }
    bool isLoaded() const {
        return Sprite::isLoaded(m_id);
    }

    Sprite& sprite();
    const Sprite& sprite() const;
    SpriteRenderer() : m_id("undefined") {
    }
    SpriteRenderer(std::string sprite_id) : m_id(sprite_id) {
        assert(isLoaded() && "texture must be first created");
    }
};
}; // namespace emp
#endif
