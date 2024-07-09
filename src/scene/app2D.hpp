#ifndef EMP_APP2D_HPP
#define EMP_APP2D_HPP
#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/model_system.hpp"
#include "graphics/renderer.hpp"

#include "io/window.hpp"
#include "math/math_defs.hpp"
#include "physics/physics_system.hpp"
#include <iostream>
#include <unordered_map>
namespace emp {
    class App2D {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        App2D();
        ~App2D();
        App2D(const App2D &) = delete;
        App2D &operator=(const App2D &) = delete;
        void run();

    private:
        Window window;
        Device device;
        Renderer renderer;
        // order of declarations matters
        std::unique_ptr<DescriptorPool> globalPool;
        std::vector<std::unique_ptr<DescriptorPool>> framePools;

        std::shared_ptr<TexturedModelsSystem> models_sys;
        std::shared_ptr<PhysicsSystem> physics_sys;
        std::shared_ptr<RigidbodySystem> rigidbody_sys;
        std::shared_ptr<ColliderSystem> collider_sys;
        std::shared_ptr<TransformSystem> transform_sys;

        std::vector<VkDescriptorSet> m_setupGlobalUBODescriptorSets(DescriptorSetLayout& globalSetLayout, const std::vector<std::unique_ptr<Buffer>>& uboBuffers);
        std::vector<std::unique_ptr<Buffer>> m_setupGlobalUBOBuffers();
        void setupECS();

        GlobalUbo m_updateUBO(FrameInfo frameinfo, Buffer& uboBuffer, Camera& camera);
        void loadAssets();
    };
}  
#endif
