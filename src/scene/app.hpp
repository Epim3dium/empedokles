#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/game_object.hpp"
#include "graphics/renderer.hpp"

#include "io/window.hpp"
#include "math/math_defs.hpp"
#include <iostream>
#include <unordered_map>
namespace emp {
    class App {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();

    private:
        Window window;
        Device device;
        Renderer renderer;
        // order of declarations matters
        std::unique_ptr<DescriptorPool> globalPool;
        std::vector<std::unique_ptr<DescriptorPool>> framePools;
        TexturedModelsManager gameObjectManager;

        std::vector<VkDescriptorSet> m_setupGlobalUBODescriptorSets(DescriptorSetLayout& globalSetLayout, const std::vector<std::unique_ptr<Buffer>>& uboBuffers);
        std::vector<std::unique_ptr<Buffer>> m_setupGlobalUBOBuffers();

        GlobalUbo m_updateUBO(FrameInfo frameinfo, Buffer& uboBuffer, Camera& camera);
        void loadGameObjects();
    };
}  
#endif
