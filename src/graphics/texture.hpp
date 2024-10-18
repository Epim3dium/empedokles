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
        return mTextureImageView;
    }

    [[nodiscard]] VkSampler sampler() const {
        return mTextureSampler;
    }

    [[nodiscard]] VkImage getImage() const {
        return mTextureImage;
    }

    [[nodiscard]] vec2f getSize() const {
        return m_size;
    }

    [[nodiscard]] VkImageView getImageView() const {
        return mTextureImageView;
    }

    [[nodiscard]] const VkDescriptorImageInfo& getImageInfo() const {
        return mDescriptor;
    }
    [[nodiscard]] VkDescriptorImageInfo& getImageInfo() {
        return mDescriptor;
    }

    [[nodiscard]] VkImageLayout getImageLayout() const {
        return mTextureLayout;
    }

    [[nodiscard]] VkExtent3D getExtent() const {
        return mExtent;
    }

    [[nodiscard]] VkFormat getFormat() const {
        return mFormat;
    }

    void updateDescriptor();

    void transitionLayout(
            VkCommandBuffer commandBuffer,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
    );

    static std::unique_ptr<TextureAsset> createTextureFromFile(
            Device& device, const std::string& filepath
    );

private:
    void createTextureImage(const std::string& filepath);

    void createTextureImageView(VkImageViewType viewType);

    void createTextureSampler();

    VkDescriptorImageInfo mDescriptor{};

    Device& mDevice;
    VkImage mTextureImage = nullptr;
    VkDeviceMemory mTextureImageMemory = nullptr;
    VkImageView mTextureImageView = nullptr;
    VkSampler mTextureSampler = nullptr;
    VkFormat mFormat;
    VkImageLayout mTextureLayout;
    uint32_t mMipLevels{1};
    uint32_t mLayerCount{1};
    VkExtent3D mExtent{};
    vec2f m_size;
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
    static void create(
            Device& device, const std::string& filepath, std::string id
    ) {
        auto tex = TextureAsset::createTextureFromFile(device, filepath);
        assert(!m_tex_table.contains(id) &&
               "trying to override existing texture id");
        m_tex_table[id] = std::move(tex);
    }
    static bool isLoaded(std::string id) {
        return m_tex_table.contains(id);
    }
    Texture() : m_id("default") {
    }
    Texture(std::string model_id) : m_id(model_id) {
        assert(isLoaded(model_id) && "texture must be first created");
    }
};

} // namespace emp

#endif
