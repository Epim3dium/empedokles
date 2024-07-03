#include "app.hpp"

#include "debug/debug.hpp"
#include "graphics/vulkan/buffer.hpp"
#include "graphics/camera.hpp"
#include "io/keyboard_movement_controller.hpp"
#include "graphics/systems/point_light_system.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "graphics/systems/simple_2D_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace emp {

    App::App():
        window{WIDTH, HEIGHT, "Vulkan MacOS M1"},
        device{window},
        renderer{window, device},
        gameObjectManager{device},
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

        loadGameObjects();
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
    void App::run() {
        auto uboBuffers = m_setupGlobalUBOBuffers();

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .build();

        auto globalDescriptorSets = m_setupGlobalUBODescriptorSets(*globalSetLayout, uboBuffers);

        EMP_LOG_DEBUG << "Alignment: " << device.properties.limits.minUniformBufferOffsetAlignment;
        EMP_LOG_DEBUG << "atom size: " << device.properties.limits.nonCoherentAtomSize;

        Simple2DColorRenderSystem simpleRenderSystem{
                device,
                renderer.getSwapChainRenderPass(),
                globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto &viewerObject = gameObjectManager.createGameObject();
        viewerObject.transform.translation.z = -2.5f;

        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!window.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                    std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.update(window.getGLFWwindow());
            cameraController.moveInPlaneXZ(frameTime, viewerObject);

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            //camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            camera.setOrthographicProjection(-1.f * aspect, 1.f * aspect, -1.f, 1.f, 0.1f, 100.f);

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                framePools[frameIndex]->resetPool();
                FrameInfo frameInfo{frameIndex,
                                    frameTime,
                                    commandBuffer,
                                    camera,
                                    globalDescriptorSets[frameIndex],
                                    *framePools[frameIndex],
                                    gameObjectManager.gameObjects};

                GlobalUbo ubo = m_updateUBO(frameInfo, *uboBuffers[frameIndex], camera);

                gameObjectManager.updateBuffer(frameIndex);
                {
                    renderer.beginSwapChainRenderPass(commandBuffer);
                    simpleRenderSystem.renderGameObjects(frameInfo);
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

    void App::loadGameObjects() {
        std::shared_ptr<Model> model =
                Model::createModelFromFile(device, "assets/models/bunny.obj");
        auto &bunny = gameObjectManager.createGameObject();
        bunny.model = model;
        bunny.transform.translation = {-.5f, .5f, 0.f};
        bunny.transform.scale = {.5f, .5f, .5f};
        bunny.transform.rotation = {0.0f, atan(1) * 4, atan(1) * 4};

        // model = Model::createModelFromFile(device, "assets/models/dragon.obj");
        // auto &dragon = gameObjectManager.createGameObject();
        // dragon.model = model;
        // dragon.transform.translation = {.5f, .2f, 0.f};
        // dragon.transform.scale = {1.f, 1.f, 1.f};
        // dragon.transform.rotation = {PI, -PI / 2, 0.0f};

        model = Model::createModelFromFile(device, "assets/models/quad.obj");
        std::shared_ptr<Texture> marbleTexture =
                Texture::createTextureFromFile(device, "../assets/textures/floor.png");
        auto &floor = gameObjectManager.createGameObject();
        floor.model = model;
        floor.diffuseMap = marbleTexture;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {6.f, 1.f, 6.f};
    }

}  // namespace emp
