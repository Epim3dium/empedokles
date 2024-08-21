#include "app.hpp"

#include "scene_defs.hpp"
#include "core/coordinator.hpp"
#include "graphics/vulkan/buffer.hpp"
#include "graphics/camera.hpp"
#include "io/keyboard_controller.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "graphics/systems/debug_shape_render_system.hpp"
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
        framePools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        auto framePoolBuilder = DescriptorPool::Builder(device)
            .setMaxSets(1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        for (auto &framePool: framePools) {
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
        coordinator.registerComponent<KeyboardController>();
        coordinator.registerComponent<Transform>();

        coordinator.registerComponent<Material>();
        coordinator.registerComponent<Collider>();
        coordinator.registerComponent<Rigidbody>();

        coordinator.registerComponent<Model>();
        coordinator.registerComponent<Texture>();
        coordinator.registerComponent<DebugShape>();

        keyboard_sys = coordinator.registerSystem<KeyboardControllerSystem>();

        transform_sys = coordinator.registerSystem<TransformSystem>();
        rigidbody_sys = coordinator.registerSystem<RigidbodySystem>();
        collider_sys  = coordinator.registerSystem<ColliderSystem>();
        physics_sys   = coordinator.registerSystem<PhysicsSystem>();

        EMP_LOG(DEBUG2) << "ECS render systems...";
        debugShape_sys = coordinator.registerSystem<DebugShapeSystem>(std::ref(device));
        models_sys    = coordinator.registerSystem<TexturedModelsSystem>(std::ref(device));
    }
    void App::run() {
        EMP_LOG(LogLevel::DEBUG) << "start running ...";
        EMP_LOG(LogLevel::DEBUG) << "ECS...";
        setupECS();

        EMP_LOG(LogLevel::DEBUG) << "assets...";
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
        SimpleRenderSystem simpleRenderSystem{
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "assets/shaders/basic_shader.vert.spv",
                "assets/shaders/basic_shader.frag.spv"};
        DebugShapeRenderSystem debugShapeRenderSystem{
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout(),
                "assets/shaders/debug_shape.vert.spv",
                "assets/shaders/debug_shape.frag.spv"};
        Camera camera{};

        auto viewerObject = coordinator.createEntity();
        coordinator.addComponent(viewerObject, Transform({0.f, 0.f}));
        // viewerObject.transform.translation.z = -2.5f;

        {
            KeyboardController cameraController{};
            cameraController.bind(eKeyMappings::MoveUp, GLFW_KEY_W);
            cameraController.bind(eKeyMappings::MoveDown, GLFW_KEY_S);
            cameraController.bind(eKeyMappings::MoveLeft, GLFW_KEY_D);
            cameraController.bind(eKeyMappings::MoveRight, GLFW_KEY_A);
            coordinator.addComponent(viewerObject, cameraController);
        }


        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                    std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            physics_sys->update(*transform_sys, *collider_sys, *rigidbody_sys, frameTime);
            transform_sys->update();
            rigidbody_sys->updateMasses();
            collider_sys->update();
            EMP_LOG_DEBUG << 1.0 / frameTime;
            keyboard_sys->update(window.getGLFWwindow());
            {
                assert(coordinator.hasComponent<Transform>(viewerObject));

                auto& viewerTransform = *coordinator.getComponent<Transform>(viewerObject);
                auto& controller = *coordinator.getComponent<KeyboardController>(viewerObject);
                viewerTransform.position += controller.movementInPlane2D() * frameTime * 2.f;

                camera.setView(viewerTransform.position, viewerTransform.rotation);
                float aspect = renderer.getAspectRatio();

#if EMP_SCENE_2D
                camera.setOrthographicProjection(-1.f * aspect, 1.f * aspect, -1.f, 1.f);
#else
                camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
#endif
            }
            onUpdate(frameTime, window);

            if (auto command_buffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                framePools[frameIndex]->resetPool();
                FrameInfo frame_info{frameIndex,
                                    frameTime,
                                    command_buffer,
                                    camera,
                                    global_descriptor_sets[frameIndex],
                                    *framePools[frameIndex]};
                                    

                GlobalUbo ubo = m_updateUBO(frame_info, *uboBuffers[frameIndex], camera);

                // models_sys->updateBuffer(frameIndex);
                debugShape_sys->updateBuffer(frameIndex);
                {
                    renderer.beginSwapChainRenderPass(command_buffer);
                    // simpleRenderSystem.render(frameInfo, *models_sys);
                    debugShapeRenderSystem.render(frame_info, *debugShape_sys);

                    onRender(device, frame_info);

                    renderer.endSwapChainRenderPass(command_buffer);
                }
                renderer.endFrame();
            }
        }

        EMP_LOG(LogLevel::DEBUG) << "destroying ECS...";
        coordinator.destroy();
        EMP_LOG(LogLevel::DEBUG) << "destroying models...";
        Model::destroyAll();
        EMP_LOG(LogLevel::DEBUG) << "destroying textures...";
        Texture::destroyAll();

        vkDeviceWaitIdle(device.device());
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
