#pragma once

#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"
#include <unordered_map>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace emp {
    class ModelAsset {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && color == other.color && normal == other.normal &&
                       uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            Builder& loadModel(const std::string &filepath);
        };

        ModelAsset(Device &device, const ModelAsset::Builder &builder);
        ~ModelAsset();
        ModelAsset(const ModelAsset &) = delete;
        ModelAsset &operator=(const ModelAsset &) = delete;

        static std::unique_ptr<ModelAsset> createModelFromFile(
                Device &device, const std::string &filepath);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer) const;
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        Device &device;

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount{};

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount{};
    };
    class Model {
    private:
        const char* m_id;
        static std::unordered_map<const char*, std::unique_ptr<ModelAsset>> m_model_table;
    public:
        const char* getID() const {return m_id; }
        ModelAsset& model() { 
            return *m_model_table.at(m_id); 
        }
        static void create(Device& device, const ModelAsset::Builder& builder, const char* id) {
            auto model = std::make_unique<ModelAsset>(device, builder);
            assert(!m_model_table.contains(id) && "trying to override existing model id");
            m_model_table[id] = std::move(model);
        }
        Model(){}
        Model(const char* model_id) : m_id(model_id) {
            assert(m_model_table.contains(m_id) && "model must be first created");
        }
    };
}  // namespace emp
