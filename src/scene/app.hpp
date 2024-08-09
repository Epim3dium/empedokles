#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "graphics/camera.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/model_system.hpp"
#include "graphics/renderer.hpp"

#include "io/keyboard_movement_controller.hpp"
#include "io/window.hpp"
#include "math/math_defs.hpp"
#include "physics/physics_system.hpp"
#include <iostream>
#include <unordered_map>
namespace emp {
    class App {
    public:

        typedef std::function<void(const float, Window&)> onUpdateFunc;
        typedef std::function<void(Device&, const FrameInfo&)> onRenderFunc;
        typedef std::function<void(Window&, Device&)> onSetup;

        App& addBehaviour(onSetup);
        App& addBehaviour(onUpdateFunc);
        App& addBehaviour(onRenderFunc);

        App& addAssetModel(std::string filename, const char* identificator);
        App& addAssetTexture(std::string filename, const char* identificator);

        template<class T, class ...InitVals>
        App& addSystem(InitVals... args) {
            to_register.push_back([&]() { coordinator.registerSystem<T>(args...); });
            return *this;
        }
        template <class T>
        App& addComponent(){
            to_register.push_front([]() { coordinator.registerComponent<T>(); });
            return *this;
        }

        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void run();

    private:
        std::vector<onSetup> on_setups;
        std::vector<onUpdateFunc> on_updates;
        std::vector<onRenderFunc> on_renders;

        struct AssetInfo {
            std::string filename;
            const char* id;
        };
        std::vector<AssetInfo> models_to_load;
        std::vector<AssetInfo> textures_to_load;

        std::deque<std::function<void(void)>> to_register;
        static constexpr int width = 800;
        static constexpr int height = 600;

        Window window;
        Device device;
        Renderer renderer;
        // order of declarations matters
        std::unique_ptr<DescriptorPool> globalPool;
        std::vector<std::unique_ptr<DescriptorPool>> framePools;

        std::shared_ptr<DebugShapeSystem> debugShape_sys;
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
