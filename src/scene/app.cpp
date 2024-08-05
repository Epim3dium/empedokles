#include "app.hpp"

#include "core/coordinator.hpp"
#include "graphics/vulkan/buffer.hpp"
#include "graphics/camera.hpp"
#include "io/keyboard_movement_controller.hpp"
#include "graphics/systems/simple_render_system.hpp"
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

    App::App():
        window{WIDTH, HEIGHT, "Vulkan MacOS M1"},
        device{window},
        renderer{window, device},
        globalPool{}
    {
        globalPool =
            DescriptorPool::Builder(device)
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
        coordinator.registerComponent<Transform>();

        coordinator.registerComponent<Material>();
        coordinator.registerComponent<Collider>();
        coordinator.registerComponent<Rigidbody>();

        coordinator.registerComponent<Model>();
        coordinator.registerComponent<Texture>();


        transform_sys = coordinator.registerSystem<TransformSystem>();
        rigidbody_sys = coordinator.registerSystem<RigidbodySystem>();
        collider_sys  = coordinator.registerSystem<ColliderSystem>();
        physics_sys   = coordinator.registerSystem<PhysicsSystem>();
        models_sys    = coordinator.registerSystem<TexturedModelsSystem>(std::ref(device));
    }
    void App::run() {
        setupECS();
        loadAssets();
        auto uboBuffers = m_setupGlobalUBOBuffers();

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .build();

        auto globalDescriptorSets = m_setupGlobalUBODescriptorSets(*globalSetLayout, uboBuffers);

        EMP_LOG_DEBUG << "Alignment: " << device.properties.limits.minUniformBufferOffsetAlignment;
        EMP_LOG_DEBUG << "atom size: " << device.properties.limits.nonCoherentAtomSize;

        SimpleRenderSystem simpleRenderSystem{
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto viewerObject = coordinator.createEntity();
        coordinator.addComponent(viewerObject, Transform({0.f, 0.f}));
        // viewerObject.transform.translation.z = -2.5f;

        KeyboardMovementController cameraController{};
        cameraController.mapping.move_up = GLFW_KEY_W;
        cameraController.mapping.move_down = GLFW_KEY_S;
        cameraController.mapping.move_left = GLFW_KEY_D;
        cameraController.mapping.move_right = GLFW_KEY_A;

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                    std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.update(window.getGLFWwindow());
            assert(coordinator.hasComponent<Transform>(viewerObject));
            auto& viewerTransform = *coordinator.findComponent<Transform>(viewerObject);
            viewerTransform.position += cameraController.movementInPlane2D() * frameTime;

            {
                camera.setView(viewerTransform.position, viewerTransform.rotation);
                float aspect = renderer.getAspectRatio();
                //camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
                camera.setOrthographicProjection(-1.f * aspect, 1.f * aspect, -1.f, 1.f);
            }
            transform_sys->update();

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                framePools[frameIndex]->resetPool();
                FrameInfo frameInfo{frameIndex,
                                    frameTime,
                                    commandBuffer,
                                    camera,
                                    globalDescriptorSets[frameIndex],
                                    *framePools[frameIndex],
                                    models_sys->entities};

                GlobalUbo ubo = m_updateUBO(frameInfo, *uboBuffers[frameIndex], camera);

                models_sys->updateBuffer(frameIndex);
                {
                    renderer.beginSwapChainRenderPass(commandBuffer);
                    simpleRenderSystem.render(frameInfo, *models_sys);
                    renderer.endSwapChainRenderPass(commandBuffer);
                }
                renderer.endFrame();
            }
        }

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
        Texture::create(device, "../assets/textures/star.jpg", "default");
        Model::create(device, ModelAsset::Builder().loadModel("../assets/models/bunny.obj"), "bunny");
        auto bunny = coordinator.createEntity();
        coordinator.addComponent(bunny, Transform(vec2f(0.f, 0.f), 0.f, {0.5f, 0.5f}));
        coordinator.addComponent(bunny, Model("bunny"));
        // coordinator.addComponent(bunny, Texture("default"));
        EMP_LOG_DEBUG << "bunny created, id: " << bunny;
    }


}  // namespace emp
