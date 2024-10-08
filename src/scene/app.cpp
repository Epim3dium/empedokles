#include "app.hpp"

#include "scene/register_scene_types.hpp"
#include "scene_defs.hpp"
#include "core/coordinator.hpp"
#include "graphics/vulkan/buffer.hpp"
#include "graphics/camera.hpp"
#include "io/keyboard_controller.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "graphics/systems/debug_shape_render_system.hpp"
#include "graphics/systems/sprite_render_system.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cassert>
#include <chrono>
#include <iostream>

namespace emp {

    App::App(std::vector<AssetInfo> models_to_load, std::vector<AssetInfo> textures_to_load):
        m_models_to_load(models_to_load),
        m_textures_to_load(textures_to_load),
        window{width, height, "Vulkan MacOS M1"},
        device{window},
        renderer{window, device},
        globalPool{}
    {
        globalPool = DescriptorPool::Builder(device)
                         .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        // build frame descriptor pools
        frame_pools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        auto framePoolBuilder = DescriptorPool::Builder(device)
            .setMaxSets(1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        for (auto &framePool: frame_pools) {
            framePool = framePoolBuilder.build();
        }

    }

    App::~App() = default;

    std::vector<VkDescriptorSet> App::m_setupGlobalUBODescriptorSets(DescriptorSetLayout& globalSetLayout, const std::vector<std::unique_ptr<Buffer>>& uboBuffers) {
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }
        return globalDescriptorSets;
    }
    std::vector<std::unique_ptr<Buffer>> App::m_setupGlobalUBOBuffers() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }
        return uboBuffers;
    }
    void App::setupECS() {
        coordinator.init();
        registerSceneTypes();
        registerSceneSystems(device);
    }
    void App::run() {
        EMP_LOG(LogLevel::DEBUG) << "start running ...";
        EMP_LOG(LogLevel::DEBUG) << "ECS...";
        setupECS();

        EMP_LOG(LogLevel::DEBUG) << "assets...";

        Sprite::init(device);
        loadAssets();
        EMP_LOG(LogLevel::DEBUG) << "users onSetup...";
        onSetup(window, device);

        EMP_LOG(LogLevel::DEBUG) << "ubo buffers...";
        auto uboBuffers = m_setupGlobalUBOBuffers();
        auto globalSetLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .build();
        auto global_descriptor_sets = m_setupGlobalUBODescriptorSets(*globalSetLayout, uboBuffers);
        EMP_LOG(DEBUG3) << "Alignment: " << device.properties.limits.minUniformBufferOffsetAlignment;
        EMP_LOG(DEBUG3) << "atom size: " << device.properties.limits.nonCoherentAtomSize;

        EMP_LOG(LogLevel::DEBUG) << "render systems...";
        m_debugShape_rend_sys = std::make_unique<DebugShapeRenderSystem>(
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "assets/shaders/debug_shape.vert.spv",
                "assets/shaders/debug_shape.frag.spv");
        m_sprite_rend_sys = std::make_unique<SpriteRenderSystem>(
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "assets/shaders/sprite.vert.spv",
                "assets/shaders/sprite.frag.spv");
        Camera camera{};

        auto viewer_object = coordinator.createEntity();
        coordinator.addComponent(viewer_object, Transform({0.f, 0.f}));
        // viewerObject.transform.translation.z = -2.5f;

        KeyboardController camera_controller{};
        camera_controller.bind(eKeyMappings::MoveUp, GLFW_KEY_W);
        camera_controller.bind(eKeyMappings::MoveDown, GLFW_KEY_S);
        camera_controller.bind(eKeyMappings::MoveLeft, GLFW_KEY_D);
        camera_controller.bind(eKeyMappings::MoveRight, GLFW_KEY_A);

        auto& physics_sys = *coordinator.getSystem<PhysicsSystem>();
        auto& transform_sys = *coordinator.getSystem<TransformSystem>();
        auto& rigidbody_sys = *coordinator.getSystem<RigidbodySystem>();
        auto& collider_sys = *coordinator.getSystem<ColliderSystem>();

        auto& debugshape_sys= *coordinator.getSystem<DebugShapeSystem>();
        auto& sprite_sys=     *coordinator.getSystem<SpriteSystem>();


        auto rendering_thread = createRenderThread(camera, global_descriptor_sets, uboBuffers);
        auto physics_thread = createPhysicsThread(120.f);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (isAppRunning) {
            std::unique_lock<std::mutex> lock(m_coordinator_access_mutex);
            while(m_isRenderer_waiting || m_isPhysics_waiting) {
                m_priority_access.wait(lock);
            }
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float delta_time =
                    std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;


            camera_controller.update(window.getGLFWwindow());
            onUpdate(delta_time, window);
            {
                assert(coordinator.hasComponent<Transform>(viewer_object));

                auto& viewer_transform = *coordinator.getComponent<Transform>(viewer_object);
                viewer_transform.position += camera_controller.movementInPlane2D() * delta_time * 2.f;

                camera.setView(viewer_transform.position, viewer_transform.rotation);
                float aspect = renderer.getAspectRatio();

#if EMP_SCENE_2D
                camera.setOrthographicProjection(-1.f * aspect, 1.f * aspect, -1.f, 1.f);
#else
                camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
#endif
            }

            isAppRunning = isAppRunning && !window.shouldClose();
            // renderFrame(camera, delta_time, global_descriptor_sets, uboBuffers);
        }
        rendering_thread->join();
        physics_thread->join();

        EMP_LOG(LogLevel::DEBUG) << "rendering thread joined";

        EMP_LOG(LogLevel::DEBUG) << "destroying ECS...";
        coordinator.destroy();
        EMP_LOG(LogLevel::DEBUG) << "destroying models...";
        Model::destroyAll();
        EMP_LOG(LogLevel::DEBUG) << "destroying textures...";
        Texture::destroyAll();

        vkDeviceWaitIdle(device.device());
    }
    std::unique_ptr<std::thread> App::createRenderThread(Camera& camera, const std::vector<VkDescriptorSet>& global_descriptor_sets, const std::vector<std::unique_ptr<Buffer>>& uboBuffers) {
        return std::move(std::make_unique<std::thread>([&]() {
                    auto current_time_render = std::chrono::high_resolution_clock::now();
                    while(isAppRunning) {
                        auto newTime = std::chrono::high_resolution_clock::now();
                        float delta_time =
                                std::chrono::duration<float, std::chrono::seconds::period>(newTime - current_time_render).count();
                        current_time_render = newTime;

                        renderFrame(camera, delta_time, global_descriptor_sets, uboBuffers);
                        EMP_LOG_INTERVAL(DEBUG2, 5.f) << "{render thread}: " << 1.f / delta_time << " FPS";
                    }
                    EMP_LOG(LogLevel::WARNING) << "rendering thread exit";
                }));
    }
    std::unique_ptr<std::thread> App::createPhysicsThread(const float target_tickrate) {
        return std::move(std::make_unique<std::thread>([&, target_tickrate]() {
            auto& physics_sys = *coordinator.getSystem<PhysicsSystem>();
            auto& transform_sys = *coordinator.getSystem<TransformSystem>();
            auto& rigidbody_sys = *coordinator.getSystem<RigidbodySystem>();
            auto& collider_sys = *coordinator.getSystem<ColliderSystem>();

            const float desired_delta_time = 1.f / target_tickrate;
            auto whole_time_physics = std::chrono::high_resolution_clock::now();
            auto last_sleep_duration = std::chrono::nanoseconds(0);
            while(isAppRunning) {
                auto newTime = std::chrono::high_resolution_clock::now();
                float delta_time =
                        std::chrono::duration<float, std::chrono::seconds::period>(newTime - whole_time_physics).count();
                float compute_delta_time =
                        std::chrono::duration<float, std::chrono::seconds::period>(newTime - whole_time_physics - last_sleep_duration).count();

                whole_time_physics = newTime;
                {
                    if(compute_delta_time < desired_delta_time) {   
                        const float sleep_duration = (desired_delta_time - compute_delta_time);
                        last_sleep_duration = std::chrono::nanoseconds( static_cast<long long>(floorf(1e9 * sleep_duration)) );
                        std::this_thread::sleep_for(last_sleep_duration);
                    }
                }
                
                m_isPhysics_waiting = true;
                std::unique_lock<std::mutex> lock(m_coordinator_access_mutex);
                while(m_isRenderer_waiting) {
                    m_priority_access.wait(lock);
                }
                m_isPhysics_waiting = false;


                physics_sys.update(transform_sys, collider_sys, rigidbody_sys, delta_time);
                EMP_LOG_INTERVAL(DEBUG2, 5.f) << "{physics thread}: " << 1.f / delta_time << " TPS";
                m_priority_access.notify_all();
            }
            EMP_LOG(LogLevel::WARNING) << "physics thread exit";
        }));
    }
    void App::renderFrame(Camera& camera, float delta_time, const std::vector<VkDescriptorSet>& global_descriptor_sets, const std::vector<std::unique_ptr<Buffer>>& uboBuffers) {
        if (auto command_buffer = renderer.beginFrame()) {

            int frame_index = renderer.getFrameIndex();
            frame_pools[frame_index]->resetPool();
            FrameInfo frame_info{frame_index,
                                delta_time,
                                command_buffer,
                                camera,
                                global_descriptor_sets[frame_index],
                                *frame_pools[frame_index]};

            {
                m_isRenderer_waiting = true;
                std::unique_lock<std::mutex> resource_lock(m_coordinator_access_mutex);
                m_isRenderer_waiting = false;

                GlobalUbo ubo = m_updateUBO(frame_info, *uboBuffers[frame_index], camera);
                auto& debugshape_sys = *coordinator.getSystem<DebugShapeSystem>();
                auto& sprite_sys = *coordinator.getSystem<SpriteSystem>();

                // models_sys->updateBuffer(frameIndex);
                debugshape_sys.updateBuffer(frame_index);
                sprite_sys.updateBuffer(frame_index);
                {
                    renderer.beginSwapChainRenderPass(command_buffer);
                    // simpleRenderSystem.render(frameInfo, *models_sys);
                    m_debugShape_rend_sys->render(frame_info, debugshape_sys);
                    m_sprite_rend_sys->render(frame_info, sprite_sys);

                    onRender(device, frame_info);

                    renderer.endSwapChainRenderPass(command_buffer);
                }
                m_priority_access.notify_all();
            }
            renderer.endFrame();
        }
    }
    GlobalUbo App::m_updateUBO(FrameInfo frameInfo, Buffer& uboBuffer, Camera& camera) {
        GlobalUbo ubo{};
        ubo.projection = camera.getProjection();
        ubo.view = camera.getView();
        ubo.inverseView = camera.getInverseView();
        uboBuffer.writeToBuffer(&ubo);
        uboBuffer.flush();
        return ubo;
    }

    void App::loadAssets() {
        for(auto tex : m_textures_to_load) {
            Texture::create(device, tex.filename, tex.id);
            EMP_LOG(LogLevel::DEBUG) << "loaded texture: " << tex.id;
        }
        m_textures_to_load.clear();
        for(auto model : m_models_to_load) {
            Model::create(device, ModelAsset::Builder().loadModel(model.filename), model.id);
            EMP_LOG(LogLevel::DEBUG) << "loaded model: " << model.id;
        }
        m_models_to_load.clear();
    }


}  // namespace emp
