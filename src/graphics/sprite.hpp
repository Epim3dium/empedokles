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
    // number of rows in texture
    int vframes = 1;
    // number of columns in texture
    int hframes = 1;
    inline int frameCount() const {
        return vframes * hframes;
    }

    int frame = 0;

    bool centered = true;

    bool flipX = false;
    bool flipY = false;
    // used for shaders stuff
    glm::vec4 color = {1, 1, 1, 1};

    bool isOverridingColor = false;
    glm::vec4 color_override = {1, 1, 1, 1};

    AABB rect() const;
    AABB shader_rect() const;
    vec2f size() const {
        return m_size;
    }
    TextureAsset& texture() {
        return m_texture.texture();
    }
    const TextureAsset& texture() const {
        return m_texture.texture();
    }
    std::string textureID() const {
        return m_texture.getID();
    }

    vec2f position_offset = vec2f(0, 0);

    // size set by default
    Sprite() { }
    Sprite(Texture tex, vec2f size = vec2f(0.f, 0.f));

    static constexpr uint32_t s_vertex_count = 6U;
    static const Vertex s_verticies[s_vertex_count];
    static std::unique_ptr<Buffer> s_vertex_buffer;

    static void init(Device& device);
    static void bind(VkCommandBuffer commandBuffer);
    static void draw(VkCommandBuffer commandBuffer);
};
}; // namespace emp
#endif
