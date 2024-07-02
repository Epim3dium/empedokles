#ifndef EMP_APP_HPP
#define EMP_APP_HPP
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
        static constexpr float PI = 3.1415926;

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
        GameObjectManager gameObjectManager;

        void loadGameObjects();
    };
}  
#endif
