#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "compute/compute_manager.hpp"
#include "graphics/camera.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/renderer.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "scene/compute_demo.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"

#include <iostream>
#include <thread>
#include "io/keyboard_controller.hpp"
#include "io/window.hpp"
#include "math/math_defs.hpp"
#include "physics/physics_system.hpp"
namespace emp {
class App {
public:
    struct AssetInfo {
        std::string filename;
        const char* id;
    };

    virtual void onUpdate(const float, Window&, KeyboardController&) {
    }
    virtual void onFixedUpdate(const float, Window&, KeyboardController&) {
    }
    virtual void onRender(Device&, const FrameInfo&) {
    }
    virtual void onSetup(Window&, Device&) {
    }

    App(std::vector<AssetInfo> models_to_load,
        std::vector<AssetInfo> textures_to_load);
    virtual ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

protected:
    static constexpr int width = 800;
    static constexpr int height = 800;
    KeyboardController controller;

    Window window;
    Device device;
    Renderer renderer;
    ComputeManager compute;
    // order of declarations matters
    std::unique_ptr<DescriptorPool> globalPool;
    std::vector<std::unique_ptr<DescriptorPool>> frame_pools;
    void forcePhysicsTickrate(const float tick_rate);

private:
    std::unique_ptr<SimpleRenderSystem> m_sprite_rend_sys;
    std::unique_ptr<SimpleRenderSystem> m_debugShape_rend_sys;
    std::unique_ptr<ComputeDemo> m_compute_demo;

    // synchronization systems
    std::condition_variable m_priority_access;
    std::mutex m_coordinator_access_mutex;
    std::atomic<bool> m_isRenderer_waiting = false;
    std::atomic<bool> m_isPhysics_waiting = false;
    std::atomic<float> m_physics_tick_rate = 60.f;
    std::atomic<bool> isAppRunning = true;

    std::vector<AssetInfo> m_models_to_load;
    std::vector<AssetInfo> m_textures_to_load;

    std::vector<VkDescriptorSet> m_setupGlobalUBODescriptorSets(
            DescriptorSetLayout& globalSetLayout,
            const std::vector<std::unique_ptr<Buffer>>& uboBuffers,
            DescriptorPool& global_pool
    );

    std::vector<std::unique_ptr<Buffer>> m_setupGlobalUBOBuffers();
    std::vector<std::unique_ptr<Buffer>> m_setupGlobalComputeUBOBuffers();

    void setupECS();

    void m_updateUBO(FrameInfo frameInfo,
        Camera& camera,
        Buffer& uboBuffer,
        Buffer& computeUboBuffer);
    void loadAssets();

    void renderFrame(
            Camera& camera,
            float delta_time,
            const std::vector<VkDescriptorSet>& global_descriptor_sets,
            const std::vector<VkDescriptorSet>& global_compute_descriptor_sets,
            const std::vector<std::unique_ptr<Buffer>>& uboBuffers,
            const std::vector<std::unique_ptr<Buffer>>& computeUboBuffers
    );
    std::unique_ptr<std::thread> createRenderThread(
            Camera& camera,
            const std::vector<VkDescriptorSet>& global_descriptor_sets,
            const std::vector<VkDescriptorSet>& global_compute_descriptor_sets,
            const std::vector<std::unique_ptr<Buffer>>& uboBuffers,
            const std::vector<std::unique_ptr<Buffer>>& comute_uboBuffers
    );
    std::unique_ptr<std::thread> createPhysicsThread();
};
} // namespace emp
#endif
