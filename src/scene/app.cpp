#include "app.hpp"
#include <ranges>

#include "core/coordinator.hpp"
#include "graphics/sprite_system.hpp"
#include "graphics/animated_sprite_system.hpp"
#include "graphics/camera.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "scene/renderer_context.hpp"
#include "vulkan/buffer.hpp"
#include "io/keyboard_controller.hpp"
#include "physics/collider.hpp"
#include "physics/rigidbody.hpp"
#include "scene/register_scene_types.hpp"
#include "scene_defs.hpp"
#include "utils/time.hpp"

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

App::App(const int w,
    const int h,
    std::vector<AssetInfo> models_to_load,
    std::vector<AssetInfo> textures_to_load)
    : m_width(w),
      m_height(h),
      m_models_to_load(models_to_load),
      m_textures_to_load(textures_to_load),
      window{w, h, "Vulkan MacOS M1"},
      controller(window.getGLFWwindow()),
      device{window},
      renderer{window, device},
      compute{device} {
}

App::~App() = default;

void App::setupECS() {
    ECS.init();
    registerSceneTypes(ECS);
    registerSceneSystems(device, ECS);
}
void App::run() {
    // Log::enableLoggingToCerr();
    Log::enableLoggingToFlie("log.log");
    EMP_LOG(LogLevel::INFO) << "start running ...";
    EMP_LOG(LogLevel::INFO) << "ECS...";
    setupECS();

    EMP_LOG(LogLevel::INFO) << "assets...";
    Sprite::init(device);
    loadAssets();
    EMP_LOG(LogLevel::INFO) << "users onSetup...";
    onSetup(window, device);

    EMP_LOG(LogLevel::INFO) << "renderer...";
    renderer_context.setup(device, renderer);

    EMP_LOG(DEBUG3) << "Alignment: "
                    << device.properties.limits.minUniformBufferOffsetAlignment;
    EMP_LOG(DEBUG3) << "atom size: "
                    << device.properties.limits.nonCoherentAtomSize;

    EMP_LOG(LogLevel::INFO) << "render systems...";

    Camera camera{};

    auto viewer_object = ECS.createEntity();
    ECS.addComponent(viewer_object, Transform({0.f, 0.f}));
    // viewerObject.transform.translation.z = -2.5f;

    auto& physics_sys = *ECS.getSystem<PhysicsSystem>();
    auto& transform_sys = *ECS.getSystem<TransformSystem>();
    auto& rigidbody_sys = *ECS.getSystem<RigidbodySystem>();
    auto& constraint_sys = *ECS.getSystem<ConstraintSystem>();
    auto& collider_sys = *ECS.getSystem<ColliderSystem>();

    auto& debugshape_sys = *ECS.getSystem<DebugShapeSystem>();
    auto& sprite_sys = *ECS.getSystem<SpriteSystem>();

#if EMP_ENABLE_RENDER_THREAD || EMP_ENABLE_PHYSICS_THREAD
    EMP_LOG(LogLevel::INFO) << "creating threads...";
#endif
#if EMP_ENABLE_RENDER_THREAD
    auto rendering_thread = createRenderThread(camera);
#endif
#if EMP_ENABLE_PHYSICS_THREAD
    auto physics_thread = createPhysicsThread();
#endif

    Stopwatch delta_clock;
    while (isAppRunning) {
#if EMP_ENABLE_RENDER_THREAD
        std::unique_lock<std::mutex> lock(m_coordinator_access_mutex);
        Stopwatch waiting_clock;
        while ((m_isRenderer_waiting || m_isPhysics_waiting) &&
               waiting_clock.getElapsedTime() < 1.f / m_physics_tick_rate) 
        {
            m_priority_access.wait(lock);
        }
#endif
        glfwPollEvents();

        float delta_time = delta_clock.restart();

        controller.update( window, *ECS.getComponent<Transform>(viewer_object));

        gui_manager.addUpdateTPS(1.f / delta_time);
        onUpdate(delta_time, window, controller);
        {
            assert(ECS.hasComponent<Transform>(viewer_object));

            auto& viewer_transform =
                    *ECS.getComponent<Transform>(viewer_object);
            viewer_transform.position +=
                    controller.lookingInPlane2D() * delta_time * 500.f;

            camera.setView(
                    viewer_transform.position, viewer_transform.rotation
            );

#if EMP_SCENE_2D
            m_width = window.getSize().width;
            m_height = window.getSize().height;
            float min_size = std::fmin(getWidth(), getHeight());

            camera.setOrthographicProjection(
                -getWidth() * 0.5f,
                getWidth() * 0.5f,
                -getHeight() * 0.5f,
                getHeight() * 0.5f);
#else
            camera.setPerspectiveProjection(
                    glm::radians(50.f), aspect, 0.1f, 100.f
            );
#endif
        }
#if not EMP_ENABLE_PHYSICS_THREAD
        physics_sys.update(
                transform_sys,
                collider_sys,
                rigidbody_sys,
                constraint_sys,
                delta_time
        );
#endif
#if not EMP_ENABLE_RENDER_THREAD
    renderFrame(camera,
        delta_time);
#endif

        isAppRunning = isAppRunning && !window.shouldClose();
    }

#if EMP_ENABLE_RENDER_THREAD
    EMP_LOG(LogLevel::DEBUG) << "rendering thread joined";
    rendering_thread->join();
#endif
#if EMP_ENABLE_PHYSICS_THREAD
    EMP_LOG(LogLevel::DEBUG) << "physics thread joined";
    physics_thread->join();
#endif

    vkDeviceWaitIdle(device.device());
    EMP_LOG(LogLevel::DEBUG) << "destroying ECS...";
    ECS.destroy();
    EMP_LOG(LogLevel::DEBUG) << "destroying models...";
    Model::destroyAll();
    EMP_LOG(LogLevel::DEBUG) << "destroying textures...";
    Texture::destroyAll();
    Sprite::s_vertex_buffer.reset();

    vkDeviceWaitIdle(device.device());
}
std::unique_ptr<std::thread> App::createRenderThread(
        Camera& camera
) {
    return std::move(std::make_unique<std::thread>([&]() {
        Stopwatch clock;
        while (isAppRunning) {
            float delta_time = clock.restart();

            renderFrame(camera,
                delta_time);
            EMP_LOG_INTERVAL(DEBUG2, 5.f)
                    << "{render thread}: " << 1.f / delta_time << " FPS";
            gui_manager.addRendererFPS(1.f / delta_time);
        }
        EMP_LOG(LogLevel::WARNING) << "rendering thread exit";
    }));
}
void App::setPhysicsTickrate(const float tick_rate) {
    m_physics_tick_rate = tick_rate;
}
std::unique_ptr<std::thread> App::createPhysicsThread() {
    return std::move(std::make_unique<std::thread>([&]() {
        auto& physics_sys = *ECS.getSystem<PhysicsSystem>();
        auto& transform_sys = *ECS.getSystem<TransformSystem>();
        auto& rigidbody_sys = *ECS.getSystem<RigidbodySystem>();
        auto& collider_sys = *ECS.getSystem<ColliderSystem>();
        auto& constraint_sys = *ECS.getSystem<ConstraintSystem>();

        Stopwatch clock;
        auto last_sleep_duration = std::chrono::nanoseconds(0);
        while (isAppRunning) {
            const float desired_delta_time = 1.f / m_physics_tick_rate;
            float delta_time = clock.restart();
            float compute_delta_time =
                    delta_time - last_sleep_duration.count() / 1e9;

            {
                if (compute_delta_time < desired_delta_time) {
                    const float sleep_duration =
                            (desired_delta_time - compute_delta_time);
                    last_sleep_duration = std::chrono::nanoseconds(
                            static_cast<long long>(floorf(1e9 * sleep_duration))
                    );
                    std::this_thread::sleep_for(last_sleep_duration);
                }
            }

            m_isPhysics_waiting = true;
            std::unique_lock<std::mutex> lock(m_coordinator_access_mutex);
            while (m_isRenderer_waiting) {
                m_priority_access.wait(lock);
            }
            m_isPhysics_waiting = false;

            onFixedUpdate(delta_time, window, controller);
            rigidbody_sys.updateMasses();
            physics_sys.update(
                    transform_sys,
                    collider_sys,
                    rigidbody_sys,
                    constraint_sys,
                    delta_time
            );
            gui_manager.addPhysicsTPS(1.f / delta_time);
            EMP_LOG_INTERVAL(DEBUG2, 5.f)
                    << "{physics thread}: " << 1.f / delta_time << " TPS";
            EMP_LOG_INTERVAL(DEBUG3, 5.f)
                    << "{physics thread}: " << 1.f / compute_delta_time
                    << " MAX TPS";

            m_priority_access.notify_all();
        }
        EMP_LOG(LogLevel::WARNING) << "physics thread exit";
    }));
}
void App::renderFrame(
        Camera& camera,
        float delta_time
) {
    auto& context = renderer_context;

    if (auto command_buffer = renderer.beginFrame()) {
        int frame_index = renderer.getFrameIndex();
        context.frame_pools[frame_index]->resetPool();
        if(auto compute_buffer = compute.beginCompute(renderer)) {
            FrameInfo frame_info{
                frame_index,
                delta_time,
                compute_buffer,
                camera,
                context.global_compute_descriptor_sets[frame_index],
                *context.frame_pools[frame_index]
            };

            renderer_context.compute_demo->performCompute(frame_info);
            compute.endCompute();
        }
        FrameInfo frame_info{
            frame_index,
            delta_time,
            command_buffer,
            camera,
            context.global_descriptor_sets[frame_index],
            *context.frame_pools[frame_index]
        };

        {
            m_isRenderer_waiting = true;
            std::unique_lock<std::mutex> resource_lock(
                    m_coordinator_access_mutex
            );
            m_isRenderer_waiting = false;

            m_updateUBO(frame_info, camera,
                *context.ubo_buffers[frame_index],
                *context.ubo_compute_buffers[frame_index]);

            auto& debugshape_sys = *ECS.getSystem<DebugShapeSystem>();
            auto& sprite_sys = *ECS.getSystem<SpriteSystem>();
            auto& animated_sprite_sys = *ECS.getSystem<AnimatedSpriteSystem>();

            // models_sys->updateBuffer(frameIndex);
            debugshape_sys.updateBuffer(frame_index);
            sprite_sys.updateBuffer(frame_index);
            animated_sprite_sys.updateTransitions(delta_time);
            animated_sprite_sys.updateBuffer(frame_index);
            {
                renderer.beginSwapChainRenderPass(command_buffer);

                debugshape_sys.render(frame_info, *renderer_context.debugShape_rend_sys);
                debugshape_sys.render(frame_info, *renderer_context.debugShapeOutline_rend_sys);
                sprite_sys.render(frame_info, *renderer_context.sprite_rend_sys);
                animated_sprite_sys.render(frame_info, *renderer_context.sprite_rend_sys);
                renderer_context.compute_demo->render(frame_info, *renderer_context.sprite_rend_sys);

                onRender(device, frame_info);
                gui_manager.draw(ECS);

                renderer.endSwapChainRenderPass(command_buffer);
            }
            m_priority_access.notify_all();
        }
        renderer.endFrame();
    }
}
void App::m_updateUBO(
        FrameInfo frameInfo, Camera& camera, Buffer& uboBuffer, Buffer& computeUboBuffer
) {
    GlobalUbo ubo{};
    ubo.projection = camera.getProjection();
    ubo.view = camera.getView();
    ubo.inverseView = camera.getInverseView();
    uboBuffer.writeToBuffer(&ubo);
    uboBuffer.flush();
    GlobalComputeUbo compute_ubo{};
    compute_ubo.delta_time = frameInfo.frameTime;
    computeUboBuffer.writeToBuffer(&ubo);
    computeUboBuffer.flush();
}

void App::loadAssets() {
    for (auto tex : m_textures_to_load) {
        try {
            Texture::create(tex.id, device, tex.filename);
            EMP_LOG(LogLevel::DEBUG) << "loaded texture: " << tex.id;
        } catch(std::runtime_error& e) {
            EMP_LOG(LogLevel::WARNING) << "failure while loading textures: " << e.what();
        } catch(...) {
            EMP_LOG(LogLevel::WARNING) << "unknown error when loading textures";
        }
    }
    m_textures_to_load.clear();
    for (auto model : m_models_to_load) {
        Model::create(
                model.id,
                device,
                ModelAsset::Builder().loadModel(model.filename)
        );
        EMP_LOG(LogLevel::DEBUG) << "loaded model: " << model.id;
    }
    m_models_to_load.clear();
}

} // namespace emp
