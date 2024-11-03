#ifndef EMP_TEXTURE_HPP
#define EMP_TEXTURE_HPP

#include "math/math_defs.hpp"
#include "vulkan/device.hpp"

// libs
#include <vulkan/vulkan.h>
#include <unordered_map>

// std
#include <memory>
#include <string>

namespace emp {
class TextureAsset {
public:
    TextureAsset(Device& device, const std::string& textureFilepath);

    TextureAsset(
            Device& device,
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage,
            VkSampleCountFlagBits sampleCount
    );

    ~TextureAsset();

    // delete copy constructors
    TextureAsset(const TextureAsset&) = delete;

    TextureAsset& operator=(const TextureAsset&) = delete;

    [[nodiscard]] VkImageView imageView() const {
        return m_texture_image_view;
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_texture_sampler;
    }

    [[nodiscard]] VkImage getImage() const {
        return m_texture_image;
    }

    [[nodiscard]] VkImageView getImageView() const {
        return m_texture_image_view;
    }

    [[nodiscard]] const VkDescriptorImageInfo& getImageInfo() const {
        return m_descriptor;
    }
    [[nodiscard]] VkDescriptorImageInfo& getImageInfo() {
        return m_descriptor;
    }

    [[nodiscard]] VkImageLayout getImageLayout() const {
        return m_texture_layout;
    }

    [[nodiscard]] VkExtent3D getExtent() const {
        return m_extent;
    }
    [[nodiscard]] vec2f getSize() const {
        return {static_cast<float>(m_extent.width), static_cast<float>(m_extent.height)};
    }

    [[nodiscard]] VkFormat getFormat() const {
        return m_format;
    }

    void updateDescriptor();

    void transitionLayout(
            VkCommandBuffer commandBuffer,
            VkImageLayout newLayout
    );
    struct Pixel {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
    };
    std::vector<Pixel> getPixelsFromGPU();

    static std::unique_ptr<TextureAsset> createTextureFromFile(
            Device& device, const std::string& filepath
    );

private:
    void createTextureImage(const std::string& filepath);
    void createTextureImageView(VkImageViewType viewType);
    void createTextureSampler();

    VkDescriptorImageInfo m_descriptor{};

    Device& m_device;
    VkImage m_texture_image = nullptr;
    VkDeviceMemory m_texture_image_memory = nullptr;
    VkImageView m_texture_image_view = nullptr;
    VkSampler m_texture_sampler = nullptr;
    VkFormat m_format;
    VkImageLayout m_texture_layout;
    uint32_t m_mip_levels{1};
    uint32_t m_layer_count{1};
    VkExtent3D m_extent{};
};
class Texture {
private:
    std::string m_id;
    static std::unordered_map<std::string, std::unique_ptr<TextureAsset>>
            m_tex_table;

public:
    static void destroyAll() {
        m_tex_table.clear();
    }
    TextureAsset& texture() {
        assert(m_tex_table.contains(m_id) &&
               "texture must be created before use");
        return *m_tex_table.at(m_id);
    }
    const TextureAsset& texture() const {
        assert(m_tex_table.contains(m_id) &&
               "texture must be created before use");
        return *m_tex_table.at(m_id);
    }
    std::string getID() const {
        return m_id;
    }
    static Texture create(
            std::string id, Device& device, const std::string& filepath
    ) {
        auto tex = TextureAsset::createTextureFromFile(device, filepath);
        assert(!m_tex_table.contains(id) &&
               "trying to override existing texture id");
        m_tex_table[id] = std::move(tex);
        return Texture(id);
    }
    static Texture create(
            std::string id, 
            Device& device,
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage,
            VkSampleCountFlagBits sampleCount
    ) {
        auto tex = std::make_unique<TextureAsset>(device, format, extent, usage, sampleCount);
        assert(!m_tex_table.contains(id) &&
               "trying to override existing texture id");
        m_tex_table[id] = std::move(tex);
        return Texture(id);
    }
    static bool isLoaded(std::string id) {
        return m_tex_table.contains(id);
    }
    Texture() : m_id("default") {
    }
    Texture(std::string model_id);
};

} // namespace emp

#endif
