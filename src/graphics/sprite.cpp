#include "sprite.hpp"
#include "debug/log.hpp"
namespace emp {
    std::unordered_map<std::string, std::unique_ptr<Sprite>> Sprite::s_sprite_table;
    std::unique_ptr<Buffer> Sprite::s_vertex_buffer;
    const Vertex Sprite::s_verticies[6] = {
        Vertex{ glm::vec3(-1.f, -1.f, 0.f), {}, {}, glm::vec2{0.f, 0.f}}, //     -'*
        Vertex{ glm::vec3( 1.f, -1.f, 0.f), {}, {}, glm::vec2{1.f, 0.f}}, //   ./  |
        Vertex{ glm::vec3( 1.f,  1.f, 0.f), {}, {}, glm::vec2{1.f, 1.f}}, //  *----*

        Vertex{ glm::vec3(-1.f, -1.f, 0.f), {}, {}, glm::vec2{0.f, 0.f}}, //  *----*
        Vertex{ glm::vec3( 1.f,  1.f, 0.f), {}, {}, glm::vec2{1.f, 1.f}}, //  |  _'
        Vertex{ glm::vec3(-1.f,  1.f, 0.f), {}, {}, glm::vec2{0.f, 1.f}}, //  *:'
    };
    void Sprite::init(Device& device) {
        assert(s_vertex_count >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(s_verticies[0]) * s_vertex_count;
        uint32_t vertexSize = sizeof(s_verticies[0]);

        Buffer stagingBuffer{
                device,
                vertexSize,
                s_vertex_count,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) s_verticies);

        s_vertex_buffer = std::make_unique<Buffer>(
                device,
                vertexSize,
                s_vertex_count,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(stagingBuffer.getBuffer(), s_vertex_buffer->getBuffer(), bufferSize);
    }
    void Sprite::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {s_vertex_buffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }
    void Sprite::draw(VkCommandBuffer commandBuffer) {
        vkCmdDraw(commandBuffer, s_vertex_count, 1, 0, 0);
    }

    Sprite::Sprite(Texture tex, AABB tex_rect, vec2f size) : m_texture(tex), m_rect(tex_rect), m_size(size) {
        auto tex_size = tex.texture().getSize();
        if(size.x == 0 && size.y == 0) {
            if(tex_size.x > tex_size.y) {
                m_size.x = 1.f;
                m_size.y = tex_size.y / tex_size.x;
            }else {
                m_size.y = 1.f;
                m_size.x = tex_size.x / tex_size.y;
            }
        }
    }
    Sprite& SpriteRenderer::sprite() {
        assert(isLoaded() && "texture must be created before use");
        return *Sprite::s_sprite_table.at(m_id);
    }
    const Sprite& SpriteRenderer::sprite() const {
        assert(isLoaded() && "texture must be created before use");
        return *Sprite::s_sprite_table.at(m_id);
    }
    Sprite& Sprite::create(std::string id, Texture tex, AABB tex_rect, vec2f size) {
        auto spr = std::make_unique<Sprite>(tex, tex_rect, size);
        assert(!isLoaded(id) && "trying to override existing texture id");
        Sprite::s_sprite_table[id] = std::move(spr);
        return *Sprite::s_sprite_table.at(id);
    }
};
